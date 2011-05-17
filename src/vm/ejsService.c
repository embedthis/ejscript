/**
    ejsService.c - Ejscript interpreter factory

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forward **********************************/

static int allocNotifier(int flags, ssize size);
static int cloneVM(Ejs *ejs, Ejs *master);
static int  configureEjs(Ejs *ejs);
static int  defineTypes(Ejs *ejs);
static void initSearchPath(Ejs *ejs, cchar *search);
static int  loadStandardModules(Ejs *ejs, MprList *require);
static void logHandler(int flags, int level, cchar *msg);
static void manageEjs(Ejs *ejs, int flags);
static void manageEjsService(EjsService *service, int flags);
static void markValues(Ejs *ejs);
static void poolTimer(EjsPool *pool, MprEvent *event);
static int  runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName);
static int  searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn);

/************************************* Code ***********************************/

static EjsService *createService()
{
    EjsService  *sp;

    if ((sp = mprAllocObj(EjsService, manageEjsService)) == NULL) {
        return 0;
    }
    MPR->ejsService = sp;
    mprSetMemNotifier((MprMemNotifier) allocNotifier);
    if (mprUsingDefaultLogHandler()) {
        ejsRedirectLogging(0);
    }
    sp->nativeModules = mprCreateHash(-1, MPR_HASH_STATIC_KEYS);
    sp->mutex = mprCreateLock();
    sp->vmlist = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    sp->vmpool = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    sp->intern = ejsCreateIntern(sp);
    ejsInitCompiler(sp);
    return sp;
}


static void manageEjsService(EjsService *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->http);
        mprMark(sp->mutex);
        mprMark(sp->vmlist);
        mprMark(sp->vmpool);
        mprMark(sp->nativeModules);
        mprMark(sp->intern);
        mprMark(sp->foundation);

    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroyIntern(sp->intern);
        sp->mutex = NULL;
    }
}


static void managePool(EjsPool *pool, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(pool->list);
        mprMark(pool->template);
        mprMark(pool->mutex);
        mprMark(pool->templateScript);
        mprMark(pool->startScriptPath);
    }
}


EjsPool *ejsCreatePool(int poolMax, cchar *templateScript, cchar *startScriptPath)
{
    EjsPool     *pool;

    if ((pool = mprAllocObj(EjsPool, managePool)) == 0) {
        return 0;
    }
    pool->mutex = mprCreateLock();
    if ((pool->list = mprCreateList(-1, 0)) == 0) {
        return 0;
    }
    pool->max = poolMax <= 0 ? MAXINT : poolMax;
    if (templateScript) {
        pool->templateScript = sclone(templateScript);
    }
    if (startScriptPath) {
        pool->startScriptPath = sclone(startScriptPath);
    }
    return pool;
}


Ejs *ejsAllocPoolVM(EjsPool *pool, int flags)
{
    Ejs         *ejs;
    EjsString   *script;

    mprAssert(pool);

    lock(pool);
    if ((ejs = mprPopItem(pool->list)) == 0) {
        if (pool->count >= pool->max) {
            mprError("Too many ejs VMS: %d max %d", pool->count, pool->max);
            unlock(pool);
            return 0;
        }
        if (pool->template == 0) {
            if ((pool->template = ejsCreateVM(0, 0, 0, 0, 0, 0, flags)) == 0) {
                return 0;
            }
            if (pool->templateScript) {
                script = ejsCreateStringFromAsc(pool->template, pool->templateScript);
                if (ejsLoadScriptLiteral(pool->template, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
                    mprError("Can't execute \"%s\"\n%s", script, ejsGetErrorMsg(pool->template, 1));
                    return 0;
                }
            }
        }
        if ((ejs = ejsCreateVM(pool->template, 0, 0, 0, 0, 0, flags)) == 0) {
            mprMemoryError("Can't alloc ejs VM");
            unlock(pool);
            return 0;
        }
        mprAddRoot(ejs);
        if (pool->startScriptPath) {
            if (ejsLoadScriptFile(ejs, pool->startScriptPath, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
                mprError("Can't load \"%s\"\n%s", pool->startScriptPath, ejsGetErrorMsg(ejs, 1));
                mprRemoveRoot(ejs);
                return 0;
            }
        }
        pool->count++;
    }
    pool->lastActivity = mprGetTime();
    mprLog(0, "ejs: Alloc VM active %d, allocated %d, max %d", pool->count - mprGetListLength(pool->list), 
        pool->count, pool->max);

    if (!mprGetDebugMode()) {
        pool->timer = mprCreateTimerEvent(NULL, "ejsPoolTimer", HTTP_TIMER_PERIOD, poolTimer, pool,
            MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
    }
    mprRemoveRoot(ejs);
    unlock(pool);
    return ejs;
}


void ejsFreePoolVM(EjsPool *pool, Ejs *ejs)
{
    mprAssert(pool);
    mprAssert(ejs);

    pool->lastActivity = mprGetTime();
    lock(pool);
    mprPushItem(pool->list, ejs);
    mprLog(0, "ejs: Free VM, active %d, allocated %d, max %d", pool->count - mprGetListLength(pool->list), pool->count,
        pool->max);
    unlock(pool);
}


static void poolTimer(EjsPool *pool, MprEvent *event)
{
    lock(pool);
    if (mprGetElapsedTime(pool->lastActivity) > EJS_POOL_INACTIVITY_TIMEOUT && !mprGetDebugMode()) {
        pool->template = 0;
        mprClearList(pool->list);
    }
    unlock(pool);
}


//  MOB - need timer to free unused VMs


#if FUTURE
//  MOB - add flag to suppress loading ejs.mod
Ejs *ejsCreateVM(cchar *search, int argc, cchar **argv, int flags)
Ejs *ejsCloneVM(Ejs *ejs)
void ejsSetDispatcher(Ejs *ejs, MprDispatcher *dispatcher);
int ejsLoadModules(Ejs *ejs, MprList *require);
#endif

//  MOB - refactor args
Ejs *ejsCreateVM(Ejs *master, MprDispatcher *dispatcher, cchar *search, MprList *require, int argc, cchar **argv, int flags)
{
    EjsService  *sp;
    Ejs         *ejs;
    static int  seqno = 0;

    if ((ejs = mprAllocObj(Ejs, manageEjs)) == NULL) {
        return 0;
    }
    mprAddRoot(ejs);

    if ((sp = MPR->ejsService) == 0) {
        sp = createService();
    }
    ejs->service = sp;
    mprAddItem(sp->vmlist, ejs);
    ejs->master = master;

    if ((ejs->state = mprAllocZeroed(sizeof(EjsState))) == 0) {
        return 0;
    }
    ejs->empty = require && mprGetListLength(require) == 0;
    ejs->mutex = mprCreateLock(ejs);
    ejs->argc = argc;
    ejs->argv = argv;
    ejs->dontExit = sp->dontExit;
    ejs->flags |= (flags & (EJS_FLAG_NO_INIT | EJS_FLAG_DOC | EJS_FLAG_HOSTED));
    ejs->hosted = (flags & EJS_FLAG_HOSTED) ? 1 : 0;

    /*
        Modules are not marked in the modules list. This way, modules are collected when not referenced.
        Workers are marked. This way workers are preserved to run in the background until they exit.
     */
    ejs->modules = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    ejs->workers = mprCreateList(0, 0);

    lock(sp);
    if (dispatcher == 0) {
        ejs->name = mprAsprintf("ejs-%d", seqno++);
        ejs->dispatcher = mprCreateDispatcher(ejs->name, 1);
    } else {
        ejs->dispatcher = dispatcher;
    }
    unlock(sp);

    if (ejsInitStack(ejs) < 0) {
        ejsDestroyVM(ejs);
        mprRemoveRoot(ejs);
        return 0;
    }
    ejs->state->frozen = 1;

    if (master) {
        if (cloneVM(ejs, master) < 0) {
            return 0;
        }
    } else {
        if (defineTypes(ejs) < 0 || loadStandardModules(ejs, require) < 0) {
            if (ejs->exception) {
                ejsReportError(ejs, "Can't initialize interpreter");
            }
            ejsDestroyVM(ejs);
            mprRemoveRoot(ejs);
            return 0;
        }
        ejsFreezeGlobal(ejs);
    }
    if (mprHasMemError(ejs)) {
        mprError("Memory allocation error during initialization");
        ejsDestroyVM(ejs);
        mprRemoveRoot(ejs);
        return 0;
    }
    initSearchPath(ejs, search);
    mprRemoveRoot(ejs);
    ejs->state->frozen = 0;
#if DEBUG_IDE
    mprLog(5, "CREATE %s, length %d", ejs->name, sp->vmlist->length);
#endif
    mprAssert(!ejs->exception);
    return ejs;
}


void ejsDestroyVM(Ejs *ejs)
{
    EjsService  *sp;
    EjsState    *state;

#if DEBUG_IDE && 0
    if (ejs->service) {
        mprLog(5, "DESTROY %s, length %d", ejs->name, ejs->service->vmlist->length);
    }
#endif
    ejs->destroying = 1;
    sp = ejs->service;
    if (sp) {
#if UNUSED
        ejsRemoveModules(ejs);
#endif
        ejsRemoveWorkers(ejs);
        state = ejs->state;
        if (state->stackBase) {
            mprVirtFree(state->stackBase, state->stackSize);
            state->stackBase = 0;
            ejs->state = 0;
        }
        mprRemoveItem(sp->vmlist, ejs);
        ejs->service = 0;
        ejs->result = 0;
        mprDestroyDispatcher(ejs->dispatcher);
    }
}


static void manageEjs(Ejs *ejs, int flags)
{
    EjsState    *start, *state;
    EjsObj      *vp, **vpp, **top;
    EjsModule   *mp;
    int         next;

    if (flags & MPR_MANAGE_MARK) {
#if DEBUG_IDE && 0
        if (ejs->service) {
            printf("MARK EJS %s, length %d", ejs->name, ejs->service->vmlist->length);
        }
#endif
        mprMark(ejs->global);
        mprMark(ejs->name);
        mprMark(ejs->doc);
        mprMark(ejs->errorMsg);
        mprMark(ejs->exception);
        mprMark(ejs->exceptionArg);
        mprMark(ejs->mutex);
        mprMark(ejs->result);
        mprMark(ejs->search);
        mprMark(ejs->dispatcher);
        mprMark(ejs->httpServers);
        mprMark(ejs->workers);

        for (next = 0; (mp = (EjsModule*) mprGetNextItem(ejs->modules, &next)) != 0;) {
            if (!mp->initialized) {
                mprMark(mp);
            }
        }
        mprMark(ejs->modules);

        /*
            Mark active call stack
         */
        start = ejs->state;
        if (start) {
            for (state = start; state; state = state->prev) {
                mprMark(state);
                mprMark(state->fp);
                mprMark(state->bp);
                mprMark(state->internal);
                mprMark(state->t1);
            }

            /*
                Mark the evaluation stack. Stack itself is virtually allocated and immune from GC.
             */
            top = start->stack;
            for (vpp = start->stackBase; vpp <= top; vpp++) {
                if ((vp = *vpp) != NULL) {
                    mprMark(vp);
                }
            }
        }
        markValues(ejs);
    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroyVM(ejs);
    }
}


static void markValues(Ejs *ejs)
{
    int     i;

    for (i = 0; i < EJS_MAX_SPECIAL; i++) {
        mprMark(ejs->values[i]);
    }
}


void ejsCloneObjHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ejs->objHelpers;
}


void ejsClonePotHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ejs->potHelpers;
    type->isPot = 1;
}


void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ST(Block)->helpers;
    type->isPot = 1;
}


static void cloneTypes(Ejs *ejs)
{
    Ejs     *master;
    
    master = (ejs->master) ? ejs->master : mprGetFirstItem(ejs->service->vmlist);
    if (master) {
        ejs->values[S_Iterator] = master->values[S_Iterator];
        ejs->values[S_StopIteration] = master->values[S_StopIteration];
    }
}


static int cloneVM(Ejs *ejs, Ejs *master)
{
    EjsAny      *vp;
    EjsModule   *mp;
    int         i, next;

    for (i = 0; i < EJS_MAX_SPECIAL; i++) {
        vp = master->values[i];
        if (vp == 0) {
            continue;
        }
#if UNUSED
        mprAssert(!((ejsIsType(ejs, vp) && ((EjsType*) vp)->mutable) ||(!ejsIsType(ejs, vp) && TYPE(vp)->mutableInstances)));
#endif
        ejs->values[i] = master->values[i];
    }
    ejs->global = master->global;

// extern int cloneCopy;
// MOB extern int cloneRef;

    ejs->global = ejsClone(ejs, master->global, 1);
// print("Copied %d, ref %d\n", cloneCopy, cloneRef);


    ejs->potHelpers = master->potHelpers;
    ejs->objHelpers = master->objHelpers;
    ejs->sqlite = master->sqlite;
    ejs->http = master->http;

    ejs->modules = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    for (next = 0; (mp = (EjsModule*) mprGetNextItem(master->modules, &next)) != 0;) {
        ejsAddModule(ejs, mp);
    }
    ejsSetPropertyByName(ejs, ejs->global, N("ejs", "global"), ejs->global);
    ejs->initialized = 1;
    return 0;
}

/*  
    Create the core language types. These are native types and are created prior to loading ejs.mod.
    The loader then matches these types to the loaded definitions.
 */
static int defineTypes(Ejs *ejs)
{
    /*  
        Create the essential bootstrap types: Object, Type and the global object, these are the foundation.
        All types are instances of Type. Order matters here.
     */
    cloneTypes(ejs);
    ejsBootstrapTypes(ejs);
    ejsCreateArrayType(ejs);
    ejsCreateBlockType(ejs);
    ejsCreateTypeType(ejs);
    ejsCreateNullType(ejs);
    ejsCreateNamespaceType(ejs);
    ejsCreateFunctionType(ejs);
    ejsCreateFrameType(ejs);
    ejsCreateGlobalBlock(ejs);

    /*
        These types are used by the compiler. Must work with an empty interpreter. Order does not matter.
     */
    ejsCreateBooleanType(ejs);
    ejsCreateErrorType(ejs);
    ejsCreateIteratorType(ejs);
    ejsCreateVoidType(ejs);
    ejsCreateNumberType(ejs);
    ejsCreatePathType(ejs);
    ejsCreateRegExpType(ejs);
    ejsCreateXMLType(ejs);
    ejsCreateXMLListType(ejs);
    ejsCreateConfigType(ejs);

    /*  
        Define the native module configuration routines.
     */
    ejsAddNativeModule(ejs, "ejs", configureEjs, _ES_CHECKSUM_ejs, 0);

#if BLD_FEATURE_EJS_ALL_IN_ONE
#if BLD_FEATURE_SQLITE
    ejs_db_sqlite_Init(ejs, NULL);
#endif
    ejs_web_Init(ejs, NULL);
#endif
    if (ejs->hasError || mprHasMemError(ejs)) {
        mprError("Can't create core types");
        return EJS_ERR;
    }
    return 0;
}


/*  
    This will configure all the core types by defining native methods and properties
 */
static int configureEjs(Ejs *ejs)
{
    /* 
        Order matters
     */
    ejsConfigureGlobalBlock(ejs);
    ejsConfigureObjectType(ejs);
    ejsConfigureIteratorType(ejs);
    ejsConfigureErrorType(ejs);
    ejsConfigureNullType(ejs);
    ejsConfigureBooleanType(ejs);
    ejsConfigureVoidType(ejs);
    ejsConfigureNumberType(ejs);

    ejsConfigurePathType(ejs);
    ejsConfigureFileSystemType(ejs);
    ejsConfigureFileType(ejs);
    ejsConfigureAppType(ejs);
    ejsConfigureArrayType(ejs);
    ejsConfigureByteArrayType(ejs);
    ejsConfigureCmdType(ejs);
    ejsConfigureDateType(ejs);
    ejsConfigureDebugType(ejs);
    ejsConfigureFunctionType(ejs);
    ejsConfigureGCType(ejs);
    ejsConfigureHttpType(ejs);
    ejsConfigureJSONType(ejs);
    ejsConfigureLogFileType(ejs);
    ejsConfigureMathType(ejs);
    ejsConfigureMemoryType(ejs);
    ejsConfigureNamespaceType(ejs);
    ejsConfigureRegExpType(ejs);
    ejsConfigureSocketType(ejs);
    ejsConfigureStringType(ejs);
    ejsConfigureSystemType(ejs);
    ejsConfigureTimerType(ejs);
    ejsConfigureUriType(ejs);
    ejsConfigureWorkerType(ejs);
    ejsConfigureXMLType(ejs);
    ejsConfigureXMLListType(ejs);

    ejsDefineConfigProperties(ejs);
    ejs->initialized = 1;
    return 0;
}


/*  
    Preload standard modules. If require is NULL, then load the standard set.
    Otherwise only load those specified in require.
 */
static int loadStandardModules(Ejs *ejs, MprList *require)
{
    char    *name;
    int     rc, next, ver, flags;

    rc = 0;
    ver = 0;
    if (require) {
        for (next = 0; rc == 0 && (name = mprGetNextItem(require, &next)) != 0; ) {
            flags = EJS_LOADER_STRICT;
            rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, name), ver, ver, flags);
        }
    } else {
        rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, "ejs"), ver, ver, EJS_LOADER_STRICT);
    }
    return rc;
}


static void initSearchPath(Ejs *ejs, cchar *search)
{
    if ((ejs->bootSearch = search) == 0) {
        ejs->bootSearch = getenv("EJSPATH");
    }
    if (ejs->bootSearch) {
        ejs->search = ejsCreateSearchPath(ejs, ejs->bootSearch);
    } else {
        ejs->search = ejsCreateSearchPath(ejs, NULL);
    }
}


void ejsSetSearchPath(Ejs *ejs, EjsArray *paths)
{
    mprAssert(ejs);
    mprAssert(paths && paths);
    mprAssert(ejsIs(ejs, paths, Array));

    ejs->search = paths;
}


EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *search)
{
    EjsArray    *ap;
    char        *relModDir, *dir, *next, *tok;

    ap = ejsCreateArray(ejs, 0);

    if (search) {
        next = sclone(search);
        dir = stok(next, MPR_SEARCH_SEP, &tok);
        while (dir && *dir) {
            ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, dir));
            dir = stok(NULL, MPR_SEARCH_SEP, &tok);
        }
        return (EjsArray*) ap;
    }
    relModDir = 0;
#if VXWORKS
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetCurrentPath(ejs)));
#else
    /*
        Create a default search path
        "." : APP_EXE_DIR/../modules : /usr/lib/ejs/1.0.0/modules
     */
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, "."));
    relModDir = mprAsprintf("%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetAppDir(ejs)));
    relModDir = mprAsprintf("%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetAbsPath(relModDir)));
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, BLD_MOD_PREFIX));
#endif
    return (EjsArray*) ap;
}


EjsObj *ejsGetGlobalObject(Ejs *ejs)
{
    return ejs->global;
}


#if FUTURE
void ejsSetServiceLocks(EjsService *sp, EjsLockFn lock, EjsUnlockFn unlock, void *data)
{
    mprAssert(sp);

    sp->lock = lock;
    sp->unlock = unlock;
    sp->lockData = data;
    return 0;
}
#endif


int ejsEvalModule(cchar *path)
{
    Ejs             *ejs;
    Mpr             *mpr;
    int             status;

    status = 0;

    if ((mpr = mprCreate(0, NULL, 0)) != 0) {
        status = MPR_ERR_MEMORY;

    } else if ((ejs = ejsCreateVM(0, 0, 0, 0, 0, 0, 0)) == 0) {
        status = MPR_ERR_MEMORY;

    } else if (ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, path), -1, -1, 0) < 0) {
        status = MPR_ERR_CANT_READ;

    } else if (ejsRun(ejs) < 0) {
        status = EJS_ERR;
    }
    mprDestroy(MPR_EXIT_DEFAULT);
    return status;
}


static int runProgram(Ejs *ejs, MprEvent *event)
{
    /*
        Run all module initialization code. This includes plain old scripts.
     */
    if (ejsRun(ejs) < 0) {
        return EJS_ERR;
    }
    if (ejs->className || ejs->methodName) {
        if (runSpecificMethod(ejs, ejs->className, ejs->methodName) < 0) {
            return EJS_ERR;
        }
    }
    return 0;
}


int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName)
{
    mprAssert(ejs->result == 0 || (MPR_GET_GEN(MPR_GET_MEM(ejs->result)) != MPR->heap.dead));

    ejs->className = className;
    ejs->methodName = methodName;
    mprRelayEvent(ejs->dispatcher, (MprEventProc) runProgram, ejs, NULL);

#if UNUSED
    if (ejs->flags & EJS_FLAG_NOEXIT) {
        /*
            If the script calls App.noexit(), this will service events until App.exit() is called.
            TODO - should deprecate noexit()
         */
        mprServiceEvents(-1, 0);
    }
#endif
    if (ejs->exception) {
        return EJS_ERR;
    }
    return 0;
}


/*  
    Run the specified method in the named class. If methodName is null, default to "main".
    If className is null, search for the first class containing the method name.
 */
static int runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName)
{
    EjsType         *type;
    EjsFunction     *fun;
    EjsArray        *args;
    int             i, slotNum;

    type = 0;
    if (className == 0 && methodName == 0) {
        return 0;
    }
    if (methodName == 0) {
        methodName = "main";
    }

    /*  
        Search for the first class with the given name
     */
    if (className == 0) {
        if (searchForMethod(ejs, methodName, &type) < 0) {
            return EJS_ERR;
        }
    } else {
        type = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, N(EJS_PUBLIC_NAMESPACE, className));
    }
    if (type == 0 || !ejsIsType(ejs, type)) {
        mprError("Can't find class \"%s\"", className);
        return EJS_ERR;
    }
    slotNum = ejsLookupProperty(ejs, type, N(EJS_PUBLIC_NAMESPACE, methodName));
    if (slotNum < 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    fun = (EjsFunction*) ejsGetProperty(ejs, type, slotNum);
    if (! ejsIsFunction(ejs, fun)) {
        mprError("Property is not a function");
        return MPR_ERR_BAD_STATE;
    }
    if (!ejsPropertyHasTrait(ejs, type, slotNum, EJS_PROP_STATIC)) {
        mprError("Method is not declared static");
        return EJS_ERR;
    }
    args = ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, ejsCreateStringFromAsc(ejs, ejs->argv[i]));
    }
    if (ejsRunFunction(ejs, fun, NULL, 1, &args) == 0) {
        return EJS_ERR;
    }
    return 0;
}


int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsFunction *observer)
{
    EjsAny      *argv[2];
    EjsObj      *emitter;
    EjsArray    *list;
    int         i;

    if (*emitterPtr == 0) {
        *emitterPtr = ejsCreateInstance(ejs, ejsGetTypeByName(ejs, N("ejs", "Emitter")), 0, NULL);
    }
    emitter = *emitterPtr;

    argv[1] = observer;
    if (ejsIs(ejs, name, Array)) {
        list = (EjsArray*) name;
        for (i = 0; i < list->length; i++) {
            name = ejsGetProperty(ejs, list, i);
            if (!ejsIs(ejs, name, Null)) {
                argv[0] = name;
                ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_on, 2, argv);
            }
        }
    } else {
        argv[0] = name;
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_on, 2, argv);
    }
    return 0;
}


#if ES_Emitter_hasObservers
int ejsHasObservers(Ejs *ejs, EjsObj *emitter)
{
    if (emitter) {
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_hasObservers, 0, NULL);
    }
    return 0;
}
#endif


int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsFunction *observer)
{
    EjsAny      *argv[2];
    EjsArray    *list;
    int         i;

    if (emitter) {
        argv[1] = observer;
        if (ejsIs(ejs, name, Array)) {
            list = (EjsArray*) name;
            for (i = 0; i < list->length; i++) {
                name = ejsGetProperty(ejs, list, i);
                if (!ejsIs(ejs, name, Null)) {
                    argv[0] = name;
                    ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_off, 2, argv);
                }
            }
        } else {
            argv[0] = name;
            ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_off, 2, argv);
        }
    }
    return 0;
}


int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *args)
{
    EjsObj  **av, **argv;
    int     i;

    if (emitter) {
        argv = args;
        av = mprAlloc((argc + 2) * sizeof(EjsObj*));
        av[0] = (EjsObj*) ejsCreateStringFromAsc(ejs, name);
        av[1] = thisObj ? thisObj : S(null);
        for (i = 0; i < argc; i++) {
            av[i + 2] = argv[i];
        }
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_fireThis, argc + 2, av);
    }
    return 0;
}


int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, EjsAny *arg)
{
    EjsObj  **argv;

    argv = (EjsObj**) &arg;
    return ejsSendEventv(ejs, emitter, name, thisObj, 1, argv);
}


/*  
    Search for the named method in all types.
 */
static int searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn)
{
    EjsFunction *method;
    EjsType     *type;
    EjsName     qname;
    EjsObj      *global, *vp;
    int         globalCount, slotNum, methodCount;
    int         methodSlot;

    mprAssert(methodName && *methodName);
    mprAssert(typeReturn);

    global = ejs->global;
    globalCount = ejsGetPropertyCount(ejs, global);

    /*  
        Search for the named method in all types
     */
    for (slotNum = 0; slotNum < globalCount; slotNum++) {
        vp = ejsGetProperty(ejs, global, slotNum);
        if (vp == 0 || !ejsIsType(ejs, vp)) {
            continue;
        }
        type = (EjsType*) vp;

        methodCount = ejsGetPropertyCount(ejs, type);

        for (methodSlot = 0; methodSlot < methodCount; methodSlot++) {
            method = (EjsFunction*) ejsGetProperty(ejs, type, methodSlot);
            if (method == 0) {
                continue;
            }

            qname = ejsGetPropertyName(ejs, type, methodSlot);
            if (qname.name && qname.name == ejsCreateStringFromAsc(ejs, methodName)) {
                *typeReturn = type;
            }
        }
    }
    return 0;
}


static void logHandler(int flags, int level, cchar *msg)
{
    char        *prefix, *tag, *amsg, lbuf[16], buf[MPR_MAX_STRING];
    static int  solo = 0;

    if (solo > 0) {
        return;
    }
    solo = 1;
    prefix = MPR->name;
    amsg = NULL;

    if (flags & MPR_WARN_SRC) {
        tag = "Warning";
    } else if (flags & MPR_ERROR_SRC) {
        tag = "Error";
    } else if (flags & MPR_FATAL_SRC) {
        tag = "Fatal";
    } else if (flags & MPR_RAW) {
        tag = NULL;
    } else {
        tag = itos(lbuf, sizeof(lbuf), level, 10);
    }
    if (tag) {
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            /* Avoid allocation if possible */
            mprSprintf(buf, sizeof(buf), "%s: %s: %s\n", prefix, tag, msg);
            msg = buf;
        } else {
            msg = amsg = mprAsprintf("%s: %s: %s\n", prefix, tag, msg);
        }
    }
    if (MPR->logFile) {
        mprFprintf(MPR->logFile, "%s", msg);
    } else {
        mprPrintfError("%s", msg);
    }
    solo = 0;
}

int ejsRedirectLogging(cchar *logSpec)
{
    MprFile     *file;
    char        *spec, *levelSpec;
    int         level;

    level = 0;
    if (logSpec == 0) {
        logSpec = "stdout:1";
    } else {
        MPR->logging = 1;
    }
    spec = sclone(logSpec);

    if ((levelSpec = strchr(spec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }
    if (strcmp(spec, "stdout") == 0) {
        file = MPR->fileSystem->stdOutput;

    } else if (strcmp(spec, "stderr") == 0) {
        file = MPR->fileSystem->stdError;

    } else {
        if ((file = mprOpenFile(spec, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
            mprPrintfError("Can't open log file %s\n", spec);
            return EJS_ERR;
        }
    }
    mprSetLogLevel(level);
    mprSetLogHandler(logHandler);
    mprSetLogFile(file);
    return 0;
}


void ejsRedirectLoggingToFile(MprFile *file, int level)
{
    if (level >= 0) {
        mprSetLogLevel(level);
    }
    if (file) {
        mprSetLogFile(file);
    }
}


//  MOB - rename to ejsPauseGC / ejsResumeGC

int ejsFreeze(Ejs *ejs, int freeze)
{
    int     old;

    old = ejs->state->frozen;
    if (freeze != -1) {
        ejs->state->frozen = freeze;
    }
    return old;
}


/*  
    Global memory allocation handler. This is invoked when there is no notifier to handle an allocation failure.
    The interpreter has an allocNotifier (see ejsService: allocNotifier) and it will handle allocation errors.
 */
static int allocNotifier(int flags, ssize size)
{
    if (flags & MPR_MEM_DEPLETED) {
        mprPrintfError("Can't allocate memory block of size %d\n", size);
        mprPrintfError("Total memory used %d\n", (int) mprGetMem());
        exit(255);

    } else if (flags & MPR_MEM_LOW) {
        mprPrintfError("Memory request for %d bytes exceeds memory red-line\n", size);
        mprPrintfError("Total memory used %d\n", (int) mprGetMem());
#if UNUSED
    } else if (flags & MPR_MEM_ATTENTION) {
        EjsService  *sp;
        Ejs         *ejs;
        int         next;
        sp = MPR->ejsService;
        lock(sp);
        for (next = 0; (ejs = mprGetNextItem(sp->vmlist, &next)) != 0; ) {
            ejs->gc = 1;
        }
        unlock(sp);
#endif
    }
    return 0;
}


#if FUTURE && KEEP
/*  
    Notifier callback function. Invoked by mprAlloc on allocation errors. This will prevent the allocation error
    bubbling up to the global memory failure handler.
 */
static void allocNotifier(int flags, uint size)
{
    /*
        This has problems. Now many interpreters and need locking to implement this.
        Possible soln - use ejsAttention.
     */
    MprAllocStats   *alloc;
    EjsObj          *argv[2], *thisObj;
    char            msg[MPR_MAX_STRING];
    va_list         dummy = NULL_INIT;

    alloc = mprGetAllocStats(ejs);
    if (granted) {
        if (ejs->memoryCallback) {
            argv[0] = ejsCreateNumber(ejs, size);
            argv[1] = ejsCreateNumber(ejs, total);
            thisObj = ejs->memoryCallback->boundThis ? ejs->memoryCallback->boundThis : ejs->global; 
            ejsRunFunction(ejs, ejs->memoryCallback, thisObj, 2, argv);
        }
        if (!ejs->exception) {
            mprSprintf(msg, sizeof(msg), "Low memory condition. Total mem: %d. Request for %d bytes granted.", 
                total, size);
            ejsCreateException(ejs, ES_MemoryError, msg, dummy);
        }
    } else {
        if (!ejs->exception) {
            mprSprintf(msg, sizeof(msg), "Memory depleted. Total mem: %d. Request for %d bytes denied.", total, size);
            ejsCreateException(ejs, ES_MemoryError, msg, dummy);
        }
    }
}
#endif




void ejsReportError(Ejs *ejs, char *fmt, ...)
{
    va_list     arg;
    cchar       *emsg;
    char        *msg, *buf;

    va_start(arg, fmt);
    
    /*  
        Compiler error format is:
        program:SEVERITY:line:errorCode:message
        Where program is either "ejsc" or "ejs"
        Where SEVERITY is either "error" or "warn"
     */
    if ((emsg = ejsGetErrorMsg(ejs, 1)) != 0) {
        msg = (char*) emsg;
        buf = 0;
    } else {
        msg = buf = mprAsprintfv(fmt, arg);
    }
    if (ejs->exception) {
        char *name = MPR->name;
        mprRawLog(0, "%s: %s\n", name, msg);
    } else {
        mprError("%s", msg);
    }
    va_end(arg);
}


void ejsLockVm(Ejs *ejs)
{
    mprLock(ejs->mutex);
}


void ejsUnlockVm(Ejs *ejs)
{
    mprUnlock(ejs->mutex);
}


void ejsLockService(Ejs *ejs)
{
    mprLock(ejs->service->mutex);
}


void ejsUnlockService(Ejs *ejs)
{
    mprUnlock(ejs->service->mutex);
}


void ejsLoadHttpService(Ejs *ejs)
{
    ejsLockService(ejs);
    if (mprGetMpr()->httpService == 0) {
        httpCreate();
    }
    ejs->http = ejs->service->http = mprGetMpr()->httpService;
    if (ejs->http == 0) {
        mprError("Can't load Http Service");
    }
    ejsUnlockService(ejs);
}


void ejsSetSpecial(Ejs *ejs, int sid, EjsAny *value)
{
    mprAssert(sid < EJS_MAX_SPECIAL);
    
    if (0 <= sid && sid < EJS_MAX_SPECIAL) {
        mprAssert(ejs->values[sid] == 0);
        ejs->values[sid] = value;
    }
}


EjsAny *ejsGetSpecial(Ejs *ejs, int sid)
{
    mprAssert(0 <= sid && sid < EJS_MAX_SPECIAL);
    return ejs->values[sid];
}


void ejsSetSpecialType(Ejs *ejs, int sid, EjsType *type)
{
    mprAssert(0 <= sid && sid < EJS_MAX_SPECIAL);
    type->sid = sid;
    ejs->values[sid] = type;
}


void ejsDisableExit(Ejs *ejs)
{
    EjsService  *sp;

    sp = MPR->ejsService;
    if (sp) {
        sp->dontExit = 1;
    }
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
