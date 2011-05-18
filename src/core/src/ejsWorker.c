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
    EjsObj      *message;
    EjsObj      *stack;
    int         callbackSlot;
} Message;

/*********************************** Forwards *********************************/

static void addWorker(Ejs *ejs, EjsWorker *worker);
static int join(Ejs *ejs, EjsObj *workers, int timeout);
static void handleError(Ejs *ejs, EjsWorker *worker, EjsObj *exception, int throwOutside);
static void loadFile(EjsWorker *insideWorker, cchar *filename);
static void removeWorker(EjsWorker *worker);
static int workerMain(EjsWorker *worker, MprEvent *event);
static EjsObj *workerPreload(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv);

/************************************ Methods *********************************/

static EjsWorker *initWorker(Ejs *ejs, EjsWorker *worker, Ejs *baseVm, cchar *name, EjsArray *search, cchar *scriptFile)
{
    Ejs             *wejs;
    EjsWorker       *self;
    EjsNamespace    *ns;
    EjsName         sname;
    static int      workerSeqno = 0;

    ejsFreeze(ejs, 1);
    if (worker == 0) {
        worker = ejsCreateWorker(ejs);
    }
    worker->ejs = ejs;
    worker->state = EJS_WORKER_BEGIN;

    if (name) {
        worker->name = sclone(name);
    } else {
        lock(ejs);
        worker->name = mprAsprintf("worker-%d", workerSeqno++);
        unlock(ejs);
    }

    /*
        Create a new interpreter and an "inside" worker object and pair it with the current "outside" worker.
        The worker interpreter gets a new dispatcher
     */
    if ((wejs = ejsCreateVM(baseVm, 0, 0, 0, 0, 0, 0)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    worker->pair = self = ejsCreateWorker(wejs);
    self->state = EJS_WORKER_BEGIN;
    self->ejs = wejs;
    self->inside = 1;
    self->pair = worker;
    self->name = sjoin("inside-", worker->name, NULL);
    mprEnableDispatcher(wejs->dispatcher);
    if (search) {
        ejsSetSearchPath(ejs, (EjsArray*) search);
    }

    //  TODO - these should be don't delete
    ejsSetProperty(ejs, worker, ES_Worker_name, ejsCreateStringFromAsc(ejs, self->name));
    ejsSetProperty(wejs, self,  ES_Worker_name, ejsCreateStringFromAsc(wejs, self->name));

    sname = ejsName(wejs, EJS_WORKER_NAMESPACE, "self");
    ejsSetPropertyByName(wejs, wejs->global, sname, self);

    /*
        Workers have a dedicated namespace to enable viewing of the worker globals (self, onmessage, postMessage...)
     */
    ns = ejsDefineReservedNamespace(wejs, wejs->global, NULL, EJS_WORKER_NAMESPACE);
    
    addWorker(ejs, worker);

    if (scriptFile) {
        worker->scriptFile = sclone(scriptFile);
        worker->state = EJS_WORKER_STARTED;
        if (mprCreateEvent(wejs->dispatcher, "workerMain", 0, (MprEventProc) workerMain, self, 0) < 0) {
            mprRemoveItem(ejs->workers, worker);
            ejsThrowStateError(ejs, "Can't create worker event");
            return 0;
        }
    }
    return worker;
}


static EjsWorker *workerConstructor(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    EjsArray    *search;
    EjsObj      *options, *value;
    cchar       *name, *scriptFile;

    ejsFreeze(ejs, 1);

    scriptFile = (argc >= 1) ? ((EjsPath*) argv[0])->value : 0;
    options = (argc == 2) ? (EjsObj*) argv[1]: NULL;
    name = 0;
    search = 0;
    if (options) {
        search = ejsGetPropertyByName(ejs, options, EN("search"));
        value = ejsGetPropertyByName(ejs, options, EN("name"));
        if (ejsIs(ejs, value, String)) {
            name = ejsToMulti(ejs, value);
        }
    }
    worker->ejs = ejs;
    worker->state = EJS_WORKER_BEGIN;
    return initWorker(ejs, worker, 0, name, search, scriptFile);
}


/*
    Add a worker object to the list of running workers for this interpreter
 */
static void addWorker(Ejs *ejs, EjsWorker *worker) 
{
    mprAssert(ejs == worker->ejs);
    mprAssert(worker);
    mprAssert(worker->state == EJS_WORKER_BEGIN);
    mprAssert(!worker->inside);

    //  OPT - locking not needed
    lock(ejs);
    mprAssert(ejs->workers->length < 10);
    mprAddItem(ejs->workers, worker);
    unlock(ejs);
}


static void removeWorker(EjsWorker *worker) 
{
    Ejs     *ejs;

    mprAssert(!worker->inside);
    mprAssert(worker);

    ejs = worker->ejs;
    if (ejs) {
        lock(ejs);
        if (ejs->workers) {
            mprRemoveItem(ejs->workers, worker);
        }
        if (ejs->joining) {
            mprSignalDispatcher(ejs->dispatcher);
        }
        /* Accelerate GC */
        if (worker->pair) {
            worker->pair->ejs->workerComplete = 1;
            worker->pair->ejs = 0;
            worker->pair->pair = 0;
            worker->pair = 0;
        }
        worker->ejs = 0;        
        unlock(ejs);
    }
}


/*
    Called when destroying ejs
 */
void ejsRemoveWorkers(Ejs *ejs)
{
    EjsWorker   *worker;
    int         next;

    for (next = 0; (worker = mprGetNextItem(ejs->workers, &next)) != NULL; ) {
        worker->ejs = 0;
    }
    ejs->workers = 0;
}


/*
    Start a worker thread. This is called by eval() and load(). Not by preload() or by Worker(). It always joins.
 */
static EjsObj *startWorker(Ejs *ejs, EjsWorker *outsideWorker, int timeout)
{
    EjsWorker   *insideWorker;
    Ejs         *inside;
    EjsString   *result;

    mprAssert(ejs);
    mprAssert(outsideWorker);
    mprAssert(!outsideWorker->inside);
    mprAssert(outsideWorker->state == EJS_WORKER_BEGIN);
    mprAssert(outsideWorker->pair);
    mprAssert(outsideWorker->pair->ejs);

    LOG(5, "Worker.startWorker");

    if (outsideWorker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    insideWorker = outsideWorker->pair;
    mprAssert(insideWorker->inside);
    mprAssert(insideWorker->state == EJS_WORKER_BEGIN);
    inside = insideWorker->ejs;

#if UNUSED
    addWorker(ejs, outsideWorker);
#endif
    outsideWorker->state = EJS_WORKER_STARTED;

    if (mprCreateEvent(inside->dispatcher, "workerMain", 0, (MprEventProc) workerMain, insideWorker, 0) < 0) {
        ejsThrowStateError(ejs, "Can't create worker event");
        return 0;
    }
    if (timeout == 0) {
        return S(undefined);
    } 
    if (timeout < 0) {
        timeout = MAXINT;
    }
    if (join(ejs, (EjsObj*) outsideWorker, timeout) < 0) {
        return S(undefined);
    }
    result = ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return S(null);
    }
    return ejsDeserialize(ejs, result);
}


/*
    function clone(deep: Boolean = null): Worker
 */
static EjsWorker *workerClone(Ejs *ejs, EjsWorker *baseWorker, int argc, EjsObj **argv)
{
    return initWorker(ejs, 0, baseWorker->pair->ejs, 0, 0, 0);
}


/*
    static function cloneSelf(): Worker
 */
static EjsWorker *workerCloneSelf(Ejs *ejs, EjsWorker *unused, int argc, EjsObj **argv)
{
    return initWorker(ejs, 0, ejs, 0, 0, 0);
}


/*
    function eval(script: String, timeout: Boolean = -1): Obj
 */
static EjsObj *workerEval(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    int     timeout;

    mprAssert(ejsIs(ejs, argv[0], String));

    worker->scriptLiteral = (EjsString*) argv[0];
    timeout = argc == 2 ? ejsGetInt(ejs, argv[1]): MAXINT;
    return startWorker(ejs, worker, timeout);
}


/*
    static function exit()
 */
static EjsObj *workerExit(Ejs *ejs, EjsWorker *unused, int argc, EjsObj **argv)
{
    /*
        Setting exiting causes the VM to suspend processing this interpreter
     */
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
    int         i, completed, joined, count;

    lock(ejs);
    completed = 0;
    joined = 0;

    if (workers == 0 || ejsIs(ejs, workers, Null)) {
        /* Join all */
        count = mprGetListLength(ejs->workers);
        for (i = 0; i < count; i++) {
            worker = mprGetItem(ejs->workers, i);
            if (worker->state >= EJS_WORKER_COMPLETE) {
                completed++;
            }
        }
        if (completed == count) {
            joined = 1;
        }
    } else if (ejsIs(ejs, workers, Array)) {
        /* Join a set */
        set = (EjsArray*) workers;
        for (i = 0; i < set->length; i++) {
            worker = (EjsWorker*) set->data[i];
            if (worker->state >= EJS_WORKER_COMPLETE) {
                completed++;
            }
        }
        if (completed == set->length) {
            joined = 1;
        }
    } else if (TYPE(workers) == ST(Worker)) {
        /* Join one worker */
        worker = (EjsWorker*) workers;
        if (worker->state >= EJS_WORKER_COMPLETE) {
            joined = 1;
        }
    }
    unlock(ejs);
    return joined;
}


static int join(Ejs *ejs, EjsObj *workers, int timeout)
{
    MprTime     mark;
    int         result, remaining;

    LOG(5, "Worker.join: joining %d", ejs->joining);
    mprAssert(!MPR->marking);
    
    mark = mprGetTime();
    remaining = timeout;
    do {
        mprAssert(!MPR->marking);
        ejs->joining = !reapJoins(ejs, workers);
        mprAssert(!MPR->marking);
        if (!ejs->joining) {
            break;
        }
        if (mprShouldAbortRequests()) {
            ejsThrowStateError(ejs, "Program instructed to exit");
            break;
        }
        mprWaitForEvent(ejs->dispatcher, remaining);
        mprAssert(!MPR->marking);
        remaining = (int) mprGetRemainingTime(mark, timeout);
        mprAssert(!MPR->marking);
    } while (remaining > 0 && !ejs->exception);
    mprAssert(!MPR->marking);

    if (ejs->exception) {
        return 0;
    }
    result = (ejs->joining) ? MPR_ERR_TIMEOUT: 0;
    ejs->joining = 0;
    LOG(7, "Worker.join: result %d", result);
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
    mprAssert(!MPR->marking);

    return (join(ejs, workers, timeout) == 0) ? S(true): S(false);
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
        ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, path), -1, -1, EJS_LOADER_RELOAD);
    }
}


/*
    function load(script: Path, timeout: Number = 0): Void
 */
static EjsObj *workerLoad(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    int     timeout;

    mprAssert(argc == 0 || ejsIs(ejs, argv[0], Path));

    worker->scriptFile = sclone(((EjsPath*) argv[0])->value);
    timeout = argc == 2 ? ejsGetInt(ejs, argv[1]): 0;
    return startWorker(ejs, worker, timeout);
}


/*
    static function lookup(name: String): Worker
 */
static EjsWorker *workerLookup(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsWorker   *worker;
    cchar       *name;
    int         next;

    name = ejsToMulti(ejs, argv[0]);
    lock(ejs);
    for (next = 0; (worker = mprGetNextItem(ejs->workers, &next)) != NULL; ) {
        if (worker->name && strcmp(name, worker->name) == 0) {
            unlock(ejs);
            return worker;
        }
    }
    unlock(ejs);
    return S(null);
}


/*
    Process a message sent from postMessage. This may run inside the worker or outside in the parent depending on the
    direction of the message. But it ALWAYS runs in the appropriate thread for the interpreter.
 */
static int doMessage(Message *msg, MprEvent *mprEvent)
{
    Ejs         *ejs;
    EjsObj      *event, *frame;
    EjsWorker   *worker;
    EjsFunction *callback;
    EjsObj      *argv[1];

    worker = msg->worker;
    worker->gotMessage = 1;
    ejs = worker->ejs;
    event = 0;
    ejsFreeze(ejs, 1);

    callback = ejsGetProperty(ejs, worker, msg->callbackSlot);

    switch (msg->callbackSlot) {
    case ES_Worker_onerror:
        event = ejsCreateObj(ejs, ST(ErrorEvent), 0);
        break;
            
    case ES_Worker_onclose:
    case ES_Worker_onmessage:
        event = ejsCreateObj(ejs, ST(Event), 0);
        break;
            
    default:
        mprAssert(msg->callbackSlot == 0);
        return 0;
    }
    worker->event = event;
    if (msg->data) {
        ejsSetProperty(ejs, event, ES_Event_data, ejsCreateStringFromAsc(ejs, msg->data));
    }
    if (msg->message) {
        ejsSetProperty(ejs, event, ES_ErrorEvent_message, msg->message);
    }
    if (msg->stack) {
        ejsSetProperty(ejs, event, ES_ErrorEvent_stack, msg->stack);
        if ((frame = ejsGetProperty(ejs, msg->stack, 0)) != 0 && !ejsIs(ejs, frame, Void)) {
            ejsSetProperty(ejs, event, ES_ErrorEvent_filename, ejsGetPropertyByName(ejs, frame, EN("filename")));
            ejsSetProperty(ejs, event, ES_ErrorEvent_lineno, ejsGetPropertyByName(ejs, frame, EN("lineno")));
        }
    }
    if (callback == 0 || ejsIs(ejs, callback, Null)) {
        if (msg->callbackSlot == ES_Worker_onmessage) {
            mprLog(6, "Discard message as no onmessage handler defined for worker");
            
        } else if (msg->callbackSlot == ES_Worker_onerror) {
            if (ejsIs(ejs, msg->message, String)) {
                ejsThrowError(ejs, "Exception in Worker: %@", ejsToString(ejs, msg->message));
            } else {
                ejsThrowError(ejs, "Exception in Worker: %s", ejsGetErrorMsg(worker->pair->ejs, 1));
            }
        } else {
            /* Ignore onclose message */
        }

    } else if (!ejsIsFunction(ejs, callback)) {
        ejsThrowTypeError(ejs, "Worker callback %s is not a function", msg->callback);

    } else {
        argv[0] = event;
        ejsRunFunction(ejs, callback, worker, 1, argv);
    }
    if (msg->callbackSlot == ES_Worker_onclose) {
        mprAssert(!worker->inside);
        worker->state = EJS_WORKER_COMPLETE;
        LOG(5, "Worker.doMessage: complete");
        /* Worker and insider interpreter are now eligible for garbage collection */
        removeWorker(worker);
    }
    mprSignalDispatcher(ejs->dispatcher);
    worker->event = 0;
    return 0;
}


/*
    function preeval(script: String): Object
    NOTE: this blocks. 
 */
static EjsObj *workerPreeval(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    Ejs         *inside;
    EjsWorker   *insideWorker;
    EjsString   *result;

    mprAssert(!worker->inside);

    if (worker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    insideWorker = worker->pair;
    mprAssert(insideWorker->inside);
    inside = insideWorker->ejs;

    (inside->service->loadScriptLiteral)(inside, (EjsString*) argv[0], NULL);
    if (inside->exception) {
        handleError(ejs, worker, inside->exception, 1);
        return 0;
    }
    result = ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return S(null);
    }
    return ejsDeserialize(ejs, result);
}


/*
    function preload(path: Path): Object
    NOTE: this blocks. 
 */
static EjsObj *workerPreload(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    Ejs         *inside;
    EjsWorker   *insideWorker;
    EjsString   *result;

    mprAssert(argc > 0 && ejsIs(ejs, argv[0], Path));
    mprAssert(!worker->inside);

    if (worker->state > EJS_WORKER_BEGIN) {
        ejsThrowStateError(ejs, "Worker has already started");
        return 0;
    }
    insideWorker = worker->pair;
    mprAssert(insideWorker->inside);
    inside = insideWorker->ejs;

    loadFile(worker->pair, ((EjsPath*) argv[0])->value);
    if (inside->exception) {
        handleError(ejs, worker, inside->exception, 1);
        return 0;
    }
    result = ejsToJSON(inside, inside->result, NULL);
    if (result == 0) {
        return S(null);
    }
    return ejsDeserialize(ejs, result);
}


static void manageMessage(Message *msg, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(msg->data);
        mprMark(msg->message);
        mprMark(msg->stack);
    }
}


static Message *createMessage()
{ 
    return mprAllocObj(Message, manageMessage);
}


/*
    Post a message to this worker. Note: the worker is the destination worker which may be the parent.

    function postMessage(data: Object, ports: Array = null): Void
 */
static EjsObj *workerPostMessage(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    EjsString       *data;
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
    ejsFreeze(ejs, 1);
    if ((data = ejsToJSON(ejs, argv[0], NULL)) == 0) {
        ejsThrowArgError(ejs, "Can't serialize message data");
        return 0;
    }
    if ((msg = createMessage()) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    target = worker->pair;
    msg->data = ejsToMulti(ejs, data);
    msg->worker = target;
    msg->callback = "onmessage";
    msg->callbackSlot = ES_Worker_onmessage;

    dispatcher = target->ejs->dispatcher;
    mprCreateEvent(dispatcher, "postMessage", 0, doMessage, msg, 0);
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
        handleError(outside, outsideWorker, inside->exception, 0);
        inside->exception = 0;
    }
    ejsFreeze(inside, 1);
    if ((msg = createMessage()) == 0) {
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
    dispatcher = outside->dispatcher;
    mprCreateEvent(dispatcher, "doMessage", 0, (MprEventProc) doMessage, msg, 0);
    return 0;
}


/*
    function terminate(): Void
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
    ejs->exiting = 1;
    mprSignalDispatcher(ejs->dispatcher);
    return 0;
}


/*
    function waitForMessage(timeout: Number = -1): Boolean
 */
static EjsBoolean *workerWaitForMessage(Ejs *ejs, EjsWorker *worker, int argc, EjsObj **argv)
{
    MprTime     mark;
    MprTime     remaining;
    int         timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]): MAXINT;
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime();
    remaining = timeout;

    worker->gotMessage = 0;
    do {
        mprWaitForEvent(ejs->dispatcher, (int) remaining);
        remaining = mprGetRemainingTime(mark, timeout);
    } while (!worker->gotMessage && remaining > 0 && !ejs->exception);

    if (worker->gotMessage) {
        worker->gotMessage = 0;
        return S(true);
    } else {
        return S(true);
    }
}


/*
    WARNING: the inside interpreter owns the exception object. Must fully extract all fields
 */
static void handleError(Ejs *ejs, EjsWorker *worker, EjsObj *exception, int throwOutside)
{
    Ejs             *inside;
    EjsString       *str;
    EjsObj          *e;
    MprDispatcher   *dispatcher;
    Message         *msg;

    mprAssert(!worker->inside);
    mprAssert(exception);
    mprAssert(ejs == worker->ejs);

    ejsFreeze(ejs, 1);
    if ((msg = createMessage()) == 0) {
        ejsThrowMemoryError(ejs);
        return;
    }
    msg->worker = worker;
    msg->callback = "onerror";
    msg->callbackSlot = ES_Worker_onerror;
    inside = worker->pair->ejs;
    
    inside->exception = 0;
    str = ejsSerialize(inside, exception, NULL);
    e = ejsDeserialize(ejs, ejsSerialize(inside, exception, NULL));
    inside->exception = exception;

    /*
        Inside interpreter owns the exception object, so must fully extract all exception. 
        Allocate into the outside worker's interpreter.
     */
    if (ejsIsError(inside, exception)) {
        msg->message = ejsGetPropertyByName(ejs, e, EN("message"));
        msg->stack = ejsGetPropertyByName(ejs, e, EN("stack"));
    } else {
        msg->message = e;
        msg->stack = 0;
    }
    if (throwOutside) {
        if (msg->stack) {
            ejsThrowStateError(ejs, "%@\n%@", ejsToString(ejs, msg->message), ejsToString(ejs, msg->stack));
        } else {
            ejsThrowStateError(ejs, "%@", ejsToString(ejs, msg->message));
        }
    }
    dispatcher = ejs->dispatcher;
    mprCreateEvent(dispatcher, "doMessage-error", 0, (MprEventProc) doMessage, msg, 0);
}


EjsWorker *ejsCreateWorker(Ejs *ejs)
{
    return ejsCreateObj(ejs, ST(Worker), 0);
}


static void manageWorker(EjsWorker *worker, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(worker->ejs);
        ejsManagePot(worker, flags);
        mprMark(worker->event);
        mprMark(worker->name);
        mprMark(worker->scriptFile);
        mprMark(worker->scriptLiteral);
        mprMark(worker->pair);

    } else if (flags & MPR_MANAGE_FREE) {
        if (!worker->inside) {
            removeWorker(worker);
        }
        if (worker->pair) {
            if (worker->pair->pair) {
                worker->pair->pair = 0;
            }
            worker->pair = 0;
        }
    }
}


void ejsConfigureWorkerType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs", "Worker"), sizeof(EjsWorker), manageWorker, EJS_POT_HELPERS);
    type->mutableInstances = 1;
    ejsSetSpecialType(ejs, S_Worker, type);
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, workerConstructor);
    ejsBindMethod(ejs, type, ES_Worker_exit, workerExit);
    ejsBindMethod(ejs, type, ES_Worker_join, workerJoin);
    ejsBindMethod(ejs, type, ES_Worker_lookup, workerLookup);
    ejsBindMethod(ejs, type, ES_Worker_cloneSelf, workerCloneSelf);
    ejsBindMethod(ejs, prototype, ES_Worker_clone, workerClone);
    ejsBindMethod(ejs, prototype, ES_Worker_eval, workerEval);
    ejsBindMethod(ejs, prototype, ES_Worker_load, workerLoad);
    ejsBindMethod(ejs, prototype, ES_Worker_preload, workerPreload);
    ejsBindMethod(ejs, prototype, ES_Worker_preeval, workerPreeval);
    ejsBindMethod(ejs, prototype, ES_Worker_postMessage, workerPostMessage);
    ejsBindMethod(ejs, prototype, ES_Worker_terminate, workerTerminate);
    ejsBindMethod(ejs, prototype, ES_Worker_waitForMessage, workerWaitForMessage);

    ejsSetSpecial(ejs, S_Event, ejsGetTypeByName(ejs, N("ejs", "Event")));
    ejsSetSpecial(ejs, S_ErrorEvent, ejsGetTypeByName(ejs, N("ejs", "ErrorEvent")));
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

