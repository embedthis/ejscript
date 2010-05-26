/*
    ejsWorker - VM Worker thread classes

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/

typedef struct Message {
    EjsWorker   *worker;
    cchar       *callback;
    char        *data;
    char        *message;
    char        *filename;
    char        *stack;
    int         lineNumber;
    int         callbackSlot;
} Message;

/*********************************** Forwards *********************************/

static void addWorker(Ejs *ejs, EjsWorker *worker);
static int join(Ejs *ejs, EjsObj *workers, int timeout);
static void handleError(Ejs *ejs, EjsWorker *worker, EjsObj *exception);
static void loadFile(EjsWorker *insideWorker, cchar *filename);
static void removeWorker(Ejs *ejs, EjsWorker *worker);
static int workerMain(EjsWorker *worker, MprEvent *event);
static EjsObj *workerPreload(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv);

/************************************ Methods *********************************/
/*
    function Worker(script: String = null, options: Object = null)

    Script is optional. If supplied, the script is run immediately by a worker thread. This call
    does not block. Options are: search and name.
 */
static EjsObj *workerConstructor(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    Ejs             *wejs;
    EjsObj          *options, *value, *search;
    EjsName         qname;
    EjsWorker       *self;
    EjsNamespace    *ns;
    cchar           *name;

    worker->ejs = ejs;
    worker->state = EJS_WORKER_BEGIN;

    options = (argc == 2) ? (EjsObj*) argv[1]: NULL;
    name = 0;
    search = 0;

    if (options) {
        search = ejsGetPropertyByName(ejs, options, ejsName(&qname, "", "search"));
        value = ejsGetPropertyByName(ejs, options, ejsName(&qname, "", "name"));
        if (ejsIsString(value)) {
            name = ejsGetString(ejs, value);
        }
    }
    if (name) {
        worker->name = mprStrdup(worker, name);
    } else {
        worker->name = mprAsprintf(worker, -1, "worker-%d", mprGetListCount(ejs->workers));
    }

    /*
        Create a new interpreter and an "inside" worker object and pair it with the current "outside" worker.
     */
    //  TODO - must change NULL to ejs to get a master clone
    wejs = ejsCreateVm(ejs->service, NULL, NULL, NULL, 0);
    if (wejs == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (search) {
        ejsSetSearchPath(ejs, (EjsArray*) search);
    }
    worker->pair = self = ejsCreateWorker(wejs);
    self->state = EJS_WORKER_BEGIN;
    self->ejs = wejs;
    self->inside = 1;
    self->pair = worker;
    self->name = mprStrcat(self, -1, "inside-", worker->name, NULL);
    mprEnableDispatcher(wejs->dispatcher);

    //  TODO - these should be don't delete
    ejsSetProperty(ejs,  (EjsObj*) worker, ES_Worker_name, (EjsObj*) ejsCreateString(ejs, self->name));
    ejsSetProperty(wejs, (EjsObj*) self,   ES_Worker_name, (EjsObj*) ejsCreateString(wejs, self->name));

    ejsSetPropertyByName(wejs, wejs->global, ejsName(&qname, EJS_WORKER_NAMESPACE, "self"), (EjsObj*) self);

    /*
        Workers have a dedicated namespace to enable viewing of the worker globals (self, onmessage, postMessage...)
     */
    ns = ejsDefineReservedNamespace(wejs, wejs->globalBlock, 0, EJS_WORKER_NAMESPACE);

    /*
        Make the inside worker permanent so we don't need to worry about whether worker->pair->ejs is valid
     */
    self->obj.permanent = 1;
    
    if (argc > 0 && ejsIsPath(ejs, argv[0])) {
        addWorker(ejs, worker);
        worker->scriptFile = mprStrdup(worker, ((EjsPath*) argv[0])->path);
        worker->state = EJS_WORKER_STARTED;
        worker->obj.permanent = 1;
        if (mprCreateEvent(wejs->dispatcher, "workerMain", 0, (MprEventProc) workerMain, self, 0) < 0) {
            ejsThrowStateError(ejs, "Can't create worker event");
            worker->obj.permanent = 0;
            return 0;
        }
    }
    return (EjsObj*) worker;
}


/*
    Add a worker object to the list of workers for this interpreter
 */
static void addWorker(Ejs *ejs, EjsWorker *worker) 
{
    mprAssert(ejs == worker->ejs);
    mprAssert(worker);
    mprAssert(!worker->inside);
    mprAssert(worker->state == EJS_WORKER_BEGIN);

    lock(ejs);
    mprAddItem(ejs->workers, worker);
    unlock(ejs);
}


static void removeWorker(Ejs *ejs, EjsWorker *worker) 
{
    mprAssert(ejs == worker->ejs);
    mprAssert(!worker->inside);
    mprAssert(worker);

    lock(ejs);
    mprRemoveItem(ejs->workers, worker);
    if (ejs->joining) {
        //  MOB - why
        mprWakeWaitService(ejs);
    }
    unlock(ejs);
}


/*
    Start a worker thread. This is called by eval() and load(). Not by preload() or by Worker(). It always joins.
 */
static EjsObj *startWorker(Ejs *ejs, EjsWorker *outsideWorker, int timeout)
{
    EjsWorker   *insideWorker;
    Ejs         *inside;
    EjsObj      *result;

    mprAssert(ejs);
    mprAssert(outsideWorker);
    mprAssert(!outsideWorker->inside);
    mprAssert(outsideWorker->state == EJS_WORKER_BEGIN);

    LOG(ejs, 5, "Worker.startWorker");

    if (outsideWorker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    mprAssert(outsideWorker->pair);
    mprAssert(outsideWorker->pair->ejs);
    insideWorker = outsideWorker->pair;
    mprAssert(insideWorker->inside);
    inside = insideWorker->ejs;
    mprAssert(insideWorker->state == EJS_WORKER_BEGIN);

    addWorker(ejs, outsideWorker);
    outsideWorker->state = EJS_WORKER_STARTED;
    outsideWorker->obj.permanent = 1;

    if (mprCreateEvent(inside->dispatcher, "workerMain", 0, (MprEventProc) workerMain, insideWorker, 0) < 0) {
        ejsThrowStateError(ejs, "Can't create worker event");
        outsideWorker->obj.permanent = 0;
        return 0;
    }
    if (timeout == 0) {
        return ejs->undefinedValue;
    } 
    if (timeout < 0) {
        timeout = MAXINT;
    }
    if (join(ejs, (EjsObj*) outsideWorker, timeout) < 0) {
        return ejs->undefinedValue;
    }
    //  MOB was "ejs"
    result = (EjsObj*) ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return ejs->nullValue;
    }
    return ejsDeserialize(ejs, (EjsString*) result);
}


/*
    function eval(script: String, timeout: Boolean = -1): String
 */
static EjsObj *workerEval(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    int     timeout;

    mprAssert(ejsIsString(argv[0]));

    worker->scriptLiteral = mprStrdup(worker, ejsGetString(ejs, argv[0]));
    timeout = argc == 2 ? ejsGetInt(ejs, argv[1]): MAXINT;
    return startWorker(ejs, worker, timeout);
}


/*
    static function exit()
 */
static EjsObj *workerExit(Ejs *ejs, EjsWorker *unused, int argc, EjsObj **argv)
{
    ejs->exiting = 1;
    ejsAttention(ejs);
    return 0;
}


/*
    Return true if the join is successful
 */
static int reapJoins(Ejs *ejs, EjsObj *workers)
{
    EjsWorker   *worker;
    EjsArray    *set;
    int         i, completed;

    lock(ejs);
    if (workers == 0 || workers == ejs->nullValue) {
        /* Join all */
        completed = 0;
        for (i = 0; i < mprGetListCount(ejs->workers); i++) {
            worker = mprGetItem(ejs->workers, i);
            if (worker->state >= EJS_WORKER_COMPLETE) {
                completed++;
            }
        }
        if (completed == mprGetListCount(ejs->workers)) {
            unlock(ejs);
            return 1;
        }
    } else if (ejsIsArray(workers)) {
        /* Join a set */
        set = (EjsArray*) workers;
        for (i = 0; i < set->length; i++) {
            worker = (EjsWorker*) set->data[i];
            if (worker->state < EJS_WORKER_COMPLETE) {
                break;
            }
        }
        if (i >= set->length) {
            unlock(ejs);
            return 1;
        }
    } else if (workers->type == ejs->workerType) {
        /* Join one worker */
        worker = (EjsWorker*) workers;
        if (worker->state >= EJS_WORKER_COMPLETE) {
            unlock(ejs);
            return 1;
        }
    }
    unlock(ejs);
    return 0;
}


static int join(Ejs *ejs, EjsObj *workers, int timeout)
{
    MprTime     mark;
    int         result, remaining;

    LOG(ejs, 5, "Worker.join: joining %d", ejs->joining);

    mark = mprGetTime(ejs);
    remaining = timeout;
    do {
        ejs->joining = !reapJoins(ejs, workers);
        if (!ejs->joining) {
            break;
        }
        ejsServiceEvents(ejs, remaining, MPR_SERVICE_ONE_THING);
        remaining = (int) mprGetRemainingTime(ejs, mark, timeout);
    } while (remaining > 0 && !ejs->exception);

    if (ejs->exception) {
        return 0;
    }
    result = (ejs->joining) ? MPR_ERR_TIMEOUT: 0;
    ejs->joining = 0;
    LOG(ejs, 5, "Worker.join: result %d", result);
    return result;
}


/*
    static function join(workers: Object = null, timeout: Number = -1): Boolean
 */
static EjsObj *workerJoin(Ejs *ejs, EjsWorker *unused, int argc, EjsObj **argv)
{
    EjsObj      *workers;
    int         timeout;

    workers = (argc > 0) ? argv[0] : NULL;
    timeout = (argc == 2) ? ejsGetInt(ejs, argv[1]) : MAXINT;

    return (join(ejs, workers, timeout) == 0) ? (EjsObj*) ejs->trueValue: (EjsObj*) ejs->falseValue;
}


/*
    Load a file into the worker. This can be a script file or a module. This runs on the inside interpreter
 */
static void loadFile(EjsWorker *worker, cchar *path)
{
    Ejs         *ejs;
    EjsObj      *result;
    cchar       *cp;

    mprAssert(worker->inside);
    mprAssert(worker->pair && worker->pair->ejs);

    ejs = worker->ejs;
    result = 0;

    if ((cp = strrchr(path, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) != 0) {
        if (ejs->service->loadScriptFile == 0) {
            ejsThrowIOError(ejs, "load: Compiling is not enabled for %s", path);
            return;
        }
        (ejs->service->loadScriptFile)(ejs, path, NULL);

    } else {
        /* Error reporting via thrown exceptions */
        ejsLoadModule(ejs, path, -1, -1, 0);
    }
}


/*
    function load(script: Path, timeout: Number = 0): Void
 */
static EjsObj *workerLoad(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    int     timeout;

    mprAssert(argc == 0 || ejsIsPath(ejs, argv[0]));

    worker->scriptFile = mprStrdup(worker, ((EjsPath*) argv[0])->path);
    timeout = argc == 2 ? ejsGetInt(ejs, argv[1]): 0;
    return startWorker(ejs, worker, timeout);
}


/*
    static function lookup(name: String): Worker
 */
static EjsObj *workerLookup(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsWorker   *worker;
    cchar       *name;
    int         next;

    name = ejsGetString(ejs, argv[0]);
    lock(ejs);
    for (next = 0; (worker = mprGetNextItem(ejs->workers, &next)) != NULL; ) {
        if (worker->name && strcmp(name, worker->name) == 0) {
            unlock(ejs);
            return (EjsObj*) worker;
        }
    }
    unlock(ejs);
    return ejs->nullValue;
}


/*
    Process a message sent from postMessage. This may run inside the worker or outside in the parent depending on the
    direction of the message. But it ALWAYS runs in the appropriate thread for the interpreter.
 */
static int doMessage(Message *msg, MprEvent *mprEvent)
{
    Ejs         *ejs;
    EjsObj      *event;
    EjsWorker   *worker;
    EjsFunction *callback;
    EjsObj      *argv[1];

    worker = msg->worker;
    worker->gotMessage = 1;
    ejs = worker->ejs;

    callback = ejsGetProperty(ejs, (EjsObj*) worker, msg->callbackSlot);

    switch (msg->callbackSlot) {
    case ES_Worker_onclose:
        event = ejsCreate(ejs, ejs->eventType, 0);
        break;
    case ES_Worker_onerror:
        event = ejsCreate(ejs, ejs->errorEventType, 0);
        break;
    case ES_Worker_onmessage:
        event = ejsCreate(ejs, ejs->eventType, 0);
        break;
    default:
        mprAssert(msg->callbackSlot == 0);
        mprFree(mprEvent);
        return 0;
    }
    if (msg->data) {
        ejsSetProperty(ejs, event, ES_Event_data, (EjsObj*) ejsCreateStringAndFree(ejs, msg->data));
    }
    if (msg->message) {
        ejsSetProperty(ejs, event, ES_ErrorEvent_message, (EjsObj*) ejsCreateStringAndFree(ejs, msg->message));
    }
    if (msg->filename) {
        ejsSetProperty(ejs, event, ES_ErrorEvent_filename, (EjsObj*) ejsCreateStringAndFree(ejs, msg->filename));
        ejsSetProperty(ejs, event, ES_ErrorEvent_lineno, (EjsObj*) ejsCreateNumber(ejs, msg->lineNumber));
    }
    if (msg->stack) {
        ejsSetProperty(ejs, event, ES_ErrorEvent_stack, (EjsObj*) ejsCreateStringAndFree(ejs, msg->stack));
    }

    if (callback == 0 || (EjsObj*) callback == ejs->nullValue) {
        if (msg->callbackSlot == ES_Worker_onmessage) {
            mprLog(ejs, 1, "Discard message as no onmessage handler defined for worker");
            
        } else if (msg->callbackSlot == ES_Worker_onerror) {
            ejsThrowError(ejs, "Exception in Worker: %s", ejsGetErrorMsg(worker->pair->ejs, 1));

        } else {
            /* Ignore onclose message */
        }

    } else if (!ejsIsFunction(callback)) {
        ejsThrowTypeError(ejs, "Worker callback %s is not a function", msg->callback);

    } else {
        argv[0] = event;
        ejsRunFunction(ejs, callback, (EjsObj*) worker, 1, argv);
    }
    if (msg->callbackSlot == ES_Worker_onclose) {
        mprAssert(!worker->inside);
        worker->state = EJS_WORKER_COMPLETE;
        LOG(ejs, 5, "Worker.doMessage: complete");
        removeWorker(ejs, worker);
        /*
            Now that the inside worker is complete, the outside worker does not need to be protected from GC
         */
        worker->obj.permanent = 0;
    }
    mprFree(msg);
    mprFree(mprEvent);
    return 0;
}


/*
    function preeval(script: String): String
    NOTE: this blocks. 
 */
static EjsObj *workerPreeval(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    Ejs         *inside;
    EjsWorker   *insideWorker;
    EjsObj      *result;

    mprAssert(!worker->inside);

    if (worker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    insideWorker = worker->pair;
    mprAssert(insideWorker->inside);
    inside = insideWorker->ejs;

    (inside->service->loadScriptLiteral)(inside, ejsGetString(ejs, argv[0]), NULL);
    if (inside->exception) {
        handleError(ejs, worker, inside->exception);
        return 0;
    }
    //  MOB - first arg was "ejs"
    result = (EjsObj*) ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return ejs->nullValue;
    }
    return ejsDeserialize(ejs, (EjsString*) result);
}


/*
    function preload(path: Path): String
    NOTE: this blocks. 
 */
static EjsObj *workerPreload(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    Ejs         *inside;
    EjsWorker   *insideWorker;
    EjsObj      *result;

    mprAssert(argc > 0 && ejsIsPath(ejs, argv[0]));
    mprAssert(!worker->inside);

    if (worker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    insideWorker = worker->pair;
    mprAssert(insideWorker->inside);
    inside = insideWorker->ejs;

    loadFile(worker->pair, ((EjsPath*) argv[0])->path);
    if (inside->exception) {
        handleError(ejs, worker, inside->exception);
        return 0;
    }
    //  MOB - first arg was "ejs"
    result = (EjsObj*) ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return ejs->nullValue;
    }
    return ejsDeserialize(ejs, (EjsString*) result);
}


/*
    Post a message to this worker. Note: the worker is the destination worker which may be the parent.
 *
    function postMessage(data: Object, ports: Array = null): Void
 */
static EjsObj *workerPostMessage(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    EjsObj          *data;
    EjsWorker       *target;
    MprDispatcher   *dispatcher;
    Message         *msg;

    if (worker->state >= EJS_WORKER_CLOSED) {
        ejsThrowStateError(ejs, "Worker has completed");
        return 0;
    }
    /*
        Create the event with serialized data in the originating interpreter. It owns the data.
     */
    if ((data = (EjsObj*) ejsToJSON(ejs, argv[0], NULL)) == 0) {
        ejsThrowArgError(ejs, "Can't serialize message data");
        return 0;
    }
    if ((msg = mprAllocObjZeroed(ejs, Message)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    target = worker->pair;
    msg->data = mprStrdup(target->ejs, ejsGetString(ejs, data));
    msg->worker = target;
    msg->callback = "onmessage";
    msg->callbackSlot = ES_Worker_onmessage;

    dispatcher = target->ejs->dispatcher;
    mprCreateEvent(dispatcher, "postMessage", 0, (MprEventProc) doMessage, msg, 0);
    return 0;
}


/*
    Worker thread main procedure. Worker is the inside worker.
 */
static int workerMain(EjsWorker *insideWorker, MprEvent *event)
{
    Ejs             *outside, *inside;
    EjsWorker       *outsideWorker;
    MprDispatcher   *dispatcher;
    Message         *msg;

    mprAssert(insideWorker->inside);
    outsideWorker = insideWorker->pair;
    mprAssert(!outsideWorker->inside);
    mprAssert(insideWorker->state == EJS_WORKER_BEGIN);

    outside = outsideWorker->ejs;
    inside = insideWorker->ejs;
    insideWorker->state = EJS_WORKER_STARTED;
    
    /*
        Run the script or file
     */
    if (outsideWorker->scriptFile) {
        loadFile(insideWorker, outsideWorker->scriptFile);

    } else if (outsideWorker->scriptLiteral) {
        if (outside->service->loadScriptLiteral == 0) {
            ejsThrowIOError(outside, "worker: Compiling is not enabled");
            return 0;
        }
        (outside->service->loadScriptLiteral)(inside, outsideWorker->scriptLiteral, NULL);
    }
    /*
        Check for exceptions
     */
    if (inside->exception) {
        handleError(outside, outsideWorker, inside->exception);
        inside->exception = 0;
    }
    if ((msg = mprAllocObjZeroed(outside, Message)) == 0) {
        ejsThrowMemoryError(outside);
        return 0;
    }

    /*
        Post "onclose" finalization message
     */
    msg->worker = outsideWorker;
    msg->callback = "onclose";
    msg->callbackSlot = ES_Worker_onclose;

    insideWorker->state = EJS_WORKER_CLOSED;
    outsideWorker->state = EJS_WORKER_CLOSED;
    insideWorker->obj.permanent = 0;
    dispatcher = outside->dispatcher;
    mprCreateEvent(dispatcher, "doMessage", 0, (MprEventProc) doMessage, msg, 0);
    return 0;
}


/*
    function terminate()
 */
static EjsObj *workerTerminate(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{    
    if (worker->state == EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has not yet started");
        return 0;
    }
    if (worker->state >= EJS_WORKER_COMPLETE) {
        return 0;
    }
  
    /*
        Switch to the inside worker if called from outside
     */
    mprAssert(worker->pair && worker->pair->ejs);
    ejs = (!worker->inside) ? worker->pair->ejs : ejs;
    worker->terminated = 1;
    ejs->exiting = 1;
    mprWakeWaitService(ejs);
    return 0;
}


/*
    function waitForMessage(timeout: Number = -1): Boolean
 */
static EjsObj *workerWaitForMessage(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    MprTime     mark;
    int         remaining, timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]): MAXINT;
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime(ejs);
    remaining = timeout;

    worker->gotMessage = 0;
    do {
        ejsServiceEvents(ejs, remaining, MPR_SERVICE_ONE_THING);
        remaining = (int) mprGetRemainingTime(ejs, mark, timeout);
    } while (!worker->gotMessage && remaining > 0 && !ejs->exception);

    if (worker->gotMessage) {
        worker->gotMessage = 0;
        return (EjsObj*) ejs->trueValue;
    } else {
        return (EjsObj*) ejs->trueValue;
    }
}


/*
    WARNING: the inside interpreter owns the exception object. Must fully extract all fields
 */
static void handleError(Ejs *ejs, EjsWorker *worker, EjsObj *exception)
{
    EjsError        *error;
    MprDispatcher   *dispatcher;
    Message         *msg;

    mprAssert(!worker->inside);
    mprAssert(exception);
    mprAssert(ejs == worker->ejs);

    if ((msg = mprAllocObjZeroed(ejs, Message)) == 0) {
        ejsThrowMemoryError(ejs);
        return;
    }
    msg->worker = worker;
    msg->callback = "onerror";
    msg->callbackSlot = ES_Worker_onerror;
    
    /*
        Inside interpreter owns the exception object, so must fully extract all exception. 
        Allocate into the outside worker's interpreter.
     */
    if (ejsIsError(exception)) {
        error = (EjsError*) exception;
        msg->message = mprStrdup(ejs, error->message);
        msg->filename = mprStrdup(ejs, error->filename ? error->filename : "script");
        msg->lineNumber = error->lineNumber;
        msg->stack = mprStrdup(ejs, error->stack);

    } else if (ejsIsString(exception)) {
        msg->message = mprStrdup(ejs, ejsGetString(ejs, exception));

    } else {
        msg->message = mprStrdup(ejs, ejsGetString(ejs, (EjsObj*) ejsToString(ejs, exception)));
    }
    dispatcher = ejs->dispatcher;
    mprCreateEvent(dispatcher, "doMessage-error", 0, (MprEventProc) doMessage, msg, 0);
}


EjsWorker *ejsCreateWorker(Ejs *ejs)
{
    return (EjsWorker*) ejsCreate(ejs, ejs->workerType, 0);
}


static void destroyWorker(Ejs *ejs, EjsWorker *worker)
{
    if (!worker->inside) {
        removeWorker(ejs, worker);
        if (worker->pair) {
            mprFree(worker->pair->ejs);
            worker->pair = 0;
        }
    } else {
        if (worker->pair && worker->pair->pair) {
            worker->pair->pair = 0;
        }
    }
    ejsFreeVar(ejs, (EjsObj*) worker, -1);
}


static void markWorker(Ejs *ejs, EjsWorker *worker)
{
    ejsMarkObject(ejs, (EjsObj*) worker);
}


void ejsConfigureWorkerType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejs->workerType = ejsConfigureNativeType(ejs, EJS_EJS_NAMESPACE, "Worker", sizeof(EjsWorker));
    type->needFinalize = 1;
    prototype = type->prototype;

    type->helpers.destroy = (EjsDestroyHelper) destroyWorker;
    type->helpers.mark = (EjsMarkHelper) markWorker;

    ejsBindConstructor(ejs, type, (EjsProc) workerConstructor);
    ejsBindMethod(ejs, type, ES_Worker_exit, (EjsProc) workerExit);
    ejsBindMethod(ejs, type, ES_Worker_join, (EjsProc) workerJoin);
    ejsBindMethod(ejs, type, ES_Worker_lookup, (EjsProc) workerLookup);

    ejsBindMethod(ejs, prototype, ES_Worker_eval, (EjsProc) workerEval);
    ejsBindMethod(ejs, prototype, ES_Worker_load, (EjsProc) workerLoad);
    ejsBindMethod(ejs, prototype, ES_Worker_preload, (EjsProc) workerPreload);
    ejsBindMethod(ejs, prototype, ES_Worker_preeval, (EjsProc) workerPreeval);
    ejsBindMethod(ejs, prototype, ES_Worker_postMessage, (EjsProc) workerPostMessage);
    ejsBindMethod(ejs, prototype, ES_Worker_terminate, (EjsProc) workerTerminate);
    ejsBindMethod(ejs, prototype, ES_Worker_waitForMessage, (EjsProc) workerWaitForMessage);
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */

