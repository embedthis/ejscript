/**
    ejsService.c - Ejscript interpreter factory

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forward **********************************/

static void cloneProperties(Ejs *ejs, Ejs *master);
static int  configureEjs(Ejs *ejs);
static void defineSharedTypes(Ejs *ejs);
static void initSearchPath(Ejs *ejs, cchar *search);
static void initStack(Ejs *ejs);
static int  loadRequiredModules(Ejs *ejs, MprList *require);
static void manageEjs(Ejs *ejs, int flags);
static void manageEjsService(EjsService *service, int flags);
static void poolTimer(EjsPool *pool, MprEvent *event);
static int  runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName);
static int  searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn);

/************************************* Code ***********************************/

static EjsService *createService()
{
    EjsService  *sp;

    if (MPR->ejsService) {
        return MPR->ejsService;
    }
    if ((sp = mprAllocObj(EjsService, manageEjsService)) == NULL) {
        return 0;
    }
    mprGlobalLock();
    MPR->ejsService = sp;
#if FUTURE && KEEP
    mprSetMemNotifier((MprMemNotifier) allocNotifier);
#endif
#if UNUSED
    if (mprUsingDefaultLogHandler()) {
        ejsRedirectLogging(0);
    }
#endif
    sp->nativeModules = mprCreateHash(-1, MPR_HASH_STATIC_KEYS);
    sp->mutex = mprCreateLock();
    sp->vmlist = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    sp->vmpool = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    sp->intern = ejsCreateIntern(sp);
    ejsInitCompiler(sp);
    mprGlobalUnlock();
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
        mprMark(sp->immutable);

    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroyIntern(sp->intern);
        sp->mutex = NULL;
    }
}



Ejs *ejsCreateVM(int argc, cchar **argv, int flags)
{
    EjsService  *sp;
    Ejs         *ejs;

    if ((ejs = mprAllocObj(Ejs, manageEjs)) == NULL) {
        return 0;
    }
    sp = ejs->service = MPR->ejsService;
    if (sp == 0) {
        sp = ejs->service = createService();
        defineSharedTypes(ejs);
    }
    ejs->empty = 1;
    ejs->state = mprAllocZeroed(sizeof(EjsState));
    ejs->argc = argc;
    ejs->argv = argv;
    ejs->name = sfmt("ejs-%d", sp->seqno++);
    ejs->dispatcher = mprCreateDispatcher(ejs->name, 1);
    ejs->mutex = mprCreateLock(ejs);
    ejs->dontExit = sp->dontExit;
    ejs->flags |= (flags & (EJS_FLAG_NO_INIT | EJS_FLAG_DOC | EJS_FLAG_HOSTED));
    ejs->hosted = (flags & EJS_FLAG_HOSTED) ? 1 : 0;

    ejs->global = ejsCreateBlock(ejs, 0);
    mprSetName(ejs->global, "global");
    ejsDefineGlobalNamespaces(ejs);

    /*
        Modules are not marked in the modules list. This way, modules are collected when not referenced.
        Workers are marked. This way workers are preserved to run in the background until they exit.
     */
    ejs->modules = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    ejs->workers = mprCreateList(0, 0);

    initStack(ejs);
    initSearchPath(ejs, 0);
    mprAddItem(sp->vmlist, ejs);

    if (ejs->hasError || mprHasMemError(ejs)) {
        ejsDestroyVM(ejs);
        mprError("Can't create VM");
        return 0;
    }
    mprLog(5, "ejs: create VM");
    return ejs;
}


Ejs *ejsCloneVM(Ejs *master)
{
    EjsModule   *mp;
    Ejs         *ejs;
    int         next;

    if (master) {
        //  MOB - cleanup
        extern int cloneCopy;
        mprAssert(!master->empty);
        cloneCopy = 0;
        if ((ejs = ejsCreateVM(master->argc, master->argv, master ? master->flags : 0)) == 0) {
            return 0;
        }
        cloneProperties(ejs, master);
        // printf("CLONE copied %d properties\n", cloneCopy);
        ejsFixTraits(ejs, ejs->global);
        ejs->sqlite = master->sqlite;
        ejs->http = master->http;
        ejs->initialized = master->initialized;
        ejs->empty = 0;
        for (next = 0; (mp = mprGetNextItem(master->modules, &next)) != 0;) {
            ejsAddModule(ejs, mp);
        }
        return ejs;
    }
    return ejsCreateVM(0, 0, 0);
}


/*
    Load the standard ejs modules with an optional override search path and list of required modules.
    If the require list is empty, then ejs->empty will be true. This routine should only be called once for an interpreter.
 */
int ejsLoadModules(Ejs *ejs, cchar *search, MprList *require)
{
    EjsService      *sp;

    sp = ejs->service;
    mprAssert(mprGetListLength(ejs->modules) == 0);

    ejs->empty = !(require == 0 || mprGetListLength(require));
    if (search) {
        initSearchPath(ejs, search);
    }
    lock(sp);
    if (loadRequiredModules(ejs, require) < 0) {
        if (ejs->exception) {
            ejsReportError(ejs, "Can't initialize interpreter");
        }
        ejsDestroyVM(ejs);
        unlock(sp);
        return MPR_ERR_CANT_READ;
    }
    unlock(sp);
    if (mprHasMemError(ejs)) {
        mprError("Memory allocation error during initialization");
        ejsDestroyVM(ejs);
        return MPR_ERR_MEMORY;
    }
    mprAssert(!ejs->exception);
    return 0;
}


void ejsDestroyVM(Ejs *ejs)
{
    EjsService  *sp;
    EjsState    *state;
    EjsModule   *mp;   

    ejs->destroying = 1;
    sp = ejs->service;
    if (sp) {
        while ((mp = mprGetFirstItem(ejs->modules)) != 0) {
            ejsRemoveModule(ejs, mp);
        }
        mprAssert(ejs->modules->length == 0);
        ejsRemoveWorkers(ejs);
        state = ejs->state;
        if (state && state->stackBase) {
            mprVirtFree(state->stackBase, state->stackSize);
            state->stackBase = 0;
            ejs->state = 0;
        }
        mprRemoveItem(sp->vmlist, ejs);
        ejs->service = 0;
        ejs->result = 0;
        if (ejs->dispatcher) {
            mprDestroyDispatcher(ejs->dispatcher);
        }
    }
    mprLog(5, "ejs: destroy VM");
}


static void manageEjs(Ejs *ejs, int flags)
{
    EjsState    *start, *state;
    EjsObj      *vp, **vpp, **top;
    EjsModule   *mp;
    int         next;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(ejs->name);
        mprMark(ejs->exception);
        mprMark(ejs->result);
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
        mprMark(ejs->service);
        mprMark(ejs->global);
        mprMark(ejs->search);
        mprMark(ejs->className);
        mprMark(ejs->methodName);
        mprMark(ejs->errorMsg);
        mprMark(ejs->hostedHome);
        mprMark(ejs->exceptionArg);
        mprMark(ejs->dispatcher);
        mprMark(ejs->workers);
        for (next = 0; (mp = mprGetNextItem(ejs->modules, &next)) != 0;) {
            if (!mp->initialized) {
                mprMark(mp);
            }
        }
        mprMark(ejs->modules);
        mprMark(ejs->httpServers);
        mprMark(ejs->doc);
        mprMark(ejs->http);
        mprMark(ejs->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroyVM(ejs);
    }
}


static void managePool(EjsPool *pool, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(pool->list);
        mprMark(pool->timer);
        mprMark(pool->mutex);
        mprMark(pool->template);
        mprMark(pool->templateScript);
        mprMark(pool->startScript);
        mprMark(pool->startScriptPath);
        mprMark(pool->hostedHome);
    }
}

/*
    Create a pool for virtual machines
 */
EjsPool *ejsCreatePool(int poolMax, cchar *templateScript, cchar *startScript, cchar *startScriptPath, char *home)
{
    EjsPool     *pool;

    if ((pool = mprAllocObj(EjsPool, managePool)) == 0) {
        return 0;
    }
    if ((pool->list = mprCreateList(-1, 0)) == 0) {
        return 0;
    }
    pool->mutex = mprCreateLock();
    pool->max = poolMax <= 0 ? MAXINT : poolMax;
    if (templateScript) {
        pool->templateScript = sclone(templateScript);
    }
    if (startScript) {
        pool->startScript = sclone(startScript);
    }
    if (startScriptPath) {
        pool->startScriptPath = sclone(startScriptPath);
    }
    if (home) {
        pool->hostedHome = sclone(home);
    }
    return pool;
}


Ejs *ejsAllocPoolVM(EjsPool *pool, int flags)
{
    Ejs         *ejs;
    EjsString   *script;
    int         paused;

    mprAssert(pool);

    if ((ejs = mprPopItem(pool->list)) == 0) {
        if (pool->count >= pool->max) {
            mprError("Too many ejs VMS: %d max %d", pool->count, pool->max);
            return 0;
        }
        lock(pool);
        if (pool->template == 0) {
            /*
                Create the pool template VM
             */
            if ((pool->template = ejsCreateVM(0, 0, flags)) == 0) {
                unlock(pool);
                return 0;
            }
            if (ejsLoadModules(pool->template, 0, 0) < 0) {
                unlock(pool);
                return 0;
            }
            if (pool->templateScript) {
                script = ejsCreateStringFromAsc(pool->template, pool->templateScript);
                paused = ejsBlockGC(pool->template);
                if (ejsLoadScriptLiteral(pool->template, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
                    mprError("Can't execute \"%@\"\n%s", script, ejsGetErrorMsg(pool->template, 1));
                    unlock(pool);
                    ejsUnblockGC(pool->template, paused);
                    return 0;
                }
                ejsUnblockGC(pool->template, paused);
            }
        }
        unlock(pool);

        if ((ejs = ejsCloneVM(pool->template)) == 0) {
            mprMemoryError("Can't alloc ejs VM");
            return 0;
        }
        if (pool->hostedHome) {
            ejs->hostedHome = pool->hostedHome;
        }
        mprAddRoot(ejs);
        if (pool->startScriptPath) {
            if (ejsLoadScriptFile(ejs, pool->startScriptPath, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
                mprError("Can't load \"%s\"\n%s", pool->startScriptPath, ejsGetErrorMsg(ejs, 1));
                mprRemoveRoot(ejs);
                return 0;
            }
        } else if (pool->startScript) {
            script = ejsCreateStringFromAsc(ejs, pool->startScript);
            if (ejsLoadScriptLiteral(ejs, script, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_BIND) < 0) {
                mprError("Can't load \"%@\"\n%s", script, ejsGetErrorMsg(ejs, 1));
                mprRemoveRoot(ejs);
                return 0;
            }
        }
        mprRemoveRoot(ejs);
        pool->count++;
    }
    pool->lastActivity = mprGetTime();
    mprLog(5, "ejs: Alloc VM active %d, allocated %d, max %d", pool->count - mprGetListLength(pool->list), 
        pool->count, pool->max);

    if (!pool->timer && !mprGetDebugMode()) {
        pool->timer = mprCreateTimerEvent(NULL, "ejsPoolTimer", EJS_POOL_INACTIVITY_TIMEOUT, poolTimer, pool,
            MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
    }
    return ejs;
}


void ejsFreePoolVM(EjsPool *pool, Ejs *ejs)
{
    mprAssert(pool);
    mprAssert(ejs);
    mprAssert(!ejs->exception);

    ejs->exception = 0;
    pool->lastActivity = mprGetTime();
    mprPushItem(pool->list, ejs);
    mprLog(5, "ejs: Free VM, active %d, allocated %d, max %d", pool->count - mprGetListLength(pool->list), pool->count,
        pool->max);
}


static void poolTimer(EjsPool *pool, MprEvent *event)
{
    if (mprGetElapsedTime(pool->lastActivity) > EJS_POOL_INACTIVITY_TIMEOUT && !mprGetDebugMode()) {
        pool->template = 0;
        mprClearList(pool->list);
        pool->count = 0;
        mprLog(5, "ejs: Release %d VMs in inactive pool. Invoking GC.", pool->count);
        mprRequestGC(MPR_FORCE_GC);
    }
}


void ejsSetDispatcher(Ejs *ejs, MprDispatcher *dispatcher)
{
    ejs->dispatcher = dispatcher;
}


void ejsApplyObjHelpers(EjsService *sp, EjsType *type)
{
    type->helpers = sp->objHelpers;
}


void ejsApplyPotHelpers(EjsService *sp, EjsType *type)
{
    type->helpers = sp->potHelpers;
    type->isPot = 1;
}


void ejsApplyBlockHelpers(EjsService *sp, EjsType *type)
{
    type->helpers = sp->blockHelpers;
    type->isPot = 1;
}


static void defineSharedTypes(Ejs *ejs)
{
    /*  
        Create the essential bootstrap types. Order matters.
     */
    ejsCreateBootstrapTypes(ejs);
    ejsCreateArrayType(ejs);
    ejsCreateNamespaceType(ejs);
    ejsCreateFunctionType(ejs);
    ejsCreateFrameType(ejs);
    ejsCreateConfigType(ejs);

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

    ejsCreateGlobalNamespaces(ejs);
    ejsAddNativeModule(ejs, "ejs", configureEjs, _ES_CHECKSUM_ejs, 0);

#if BLD_FEATURE_EJSRIPT_ALL_IN_ONE
#if BLD_FEATURE_SQLITE
    ejs_db_sqlite_Init(ejs, NULL);
#endif
    ejs_web_Init(ejs, NULL);
#endif
}


static void cloneProperties(Ejs *ejs, Ejs *master)
{
    EjsAny      *vp, *mvp;
    EjsName     qname;
    EjsTrait    *trait;
    int         i, immutable, numProp;

    mprAssert(ejs);
    mprAssert(master);

    /*
        For subsequent VMs, copy global references to immutable types and functions.
     */
    numProp = ((EjsPot*) master->global)->numProp;
    for (i = 0; i < numProp; i++) {
        vp = ejsGetProperty(master, master->global, i);
        qname = ejsGetPropertyName(master, master->global, i);
        ejsSetPropertyName(ejs, ejs->global, i, qname);
        trait = ejsGetPropertyTraits(master, master->global, i);
        ejsSetPropertyTraits(ejs, ejs->global, i, trait->type, trait->attributes);
        immutable = 0;
        if (ejsIsType(ejs, vp)) {
            if (!((EjsType*) vp)->mutable) {
                immutable = 1;
            }
        } else if (!TYPE(vp)->mutableInstances || ejsIsFunction(ejs, vp)) {
            immutable = 1;
        } else if (vp == master->global) {
            vp = ejs->global;
            immutable = 1;
        }
        if (!immutable) {
            mvp = vp;
            vp = ejsClone(ejs, mvp, 1);
        }
        ejsSetProperty(ejs, ejs->global, i, vp);
    }
}


static void initStack(Ejs *ejs)
{
    EjsState    *state;

    /*
        This will allocate memory virtually for systems with virutal memory. Otherwise, it will just use malloc.
     */
    state = ejs->state;
    state->stackSize = MPR_PAGE_ALIGN(EJS_STACK_MAX, mprGetPageSize(ejs));
    if ((state->stackBase = mprVirtAlloc(state->stackSize, MPR_MAP_READ | MPR_MAP_WRITE)) != 0) {
        state->stack = &state->stackBase[-1];
    }
}


/*  
    This will configure all the core types by defining native methods and properties
    This runs after ejs.mod is loaded. NOTE: this does not happen when compiling ejs.mod (ejs->empty).
 */
static int configureEjs(Ejs *ejs)
{
    if (!ejs->service->immutableInitialized) {
        /* 
            Configure shared immutable types
         */
        ejsConfigureIteratorType(ejs);
        ejsConfigureErrorType(ejs);
        ejsConfigureNullType(ejs);
        ejsConfigureBooleanType(ejs);
        ejsConfigureVoidType(ejs);
        ejsConfigureNumberType(ejs);

        ejsConfigurePathType(ejs);
        ejsConfigureFileSystemType(ejs);
        ejsConfigureFileType(ejs);
        ejsConfigureArrayType(ejs);
        ejsConfigureByteArrayType(ejs);
        ejsConfigureCmdType(ejs);
        ejsConfigureDateType(ejs);
        ejsConfigureFunctionType(ejs);
        ejsConfigureGCType(ejs);
        ejsConfigureHttpType(ejs);
        ejsConfigureLocalCacheType(ejs);
        ejsConfigureMprLogType(ejs);
        ejsConfigureMathType(ejs);
        ejsConfigureMemoryType(ejs);
        ejsConfigureNamespaceType(ejs);
        ejsConfigureRegExpType(ejs);
        ejsConfigureSocketType(ejs);
        ejsConfigureStringType(ejs);
        ejsConfigureSystemType(ejs);
        ejsConfigureTimerType(ejs);
        ejsConfigureWorkerType(ejs);
        ejsConfigureXMLType(ejs);
        ejsConfigureXMLListType(ejs);
        ejs->service->immutableInitialized = 1;
    }

    /*
        These types have global properties that must be initialized for all interpreters
     */
    ejsConfigureObjectType(ejs);
    ejsConfigureDebugType(ejs);
    ejsConfigureJSONType(ejs);
    ejsConfigureUriType(ejs);

    /*
        Configure mutables
     */
    ejsConfigureGlobalBlock(ejs);
    ejsConfigureAppType(ejs);
    ejsDefineConfigProperties(ejs);
    ejs->initialized = 1;
    return 0;
}


/*  
    Preload required modules. If require is NULL, then load the standard set.
    Otherwise only load those specified in require.
 */
static int loadRequiredModules(Ejs *ejs, MprList *require)
{
    char    *name;
    int     rc, next, paused;

    rc = 0;
    paused = ejsBlockGC(ejs);
    if (require) {
        for (next = 0; rc == 0 && (name = mprGetNextItem(require, &next)) != 0; ) {
            rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, name), 0, 0, EJS_LOADER_STRICT);
        }
    } else {
        rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, "ejs"), 0, 0, EJS_LOADER_STRICT);
    }
    ejsFreezeGlobal(ejs);
    ejsUnblockGC(ejs, paused);
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
        "." : APP_EXE_DIR/../lib : /usr/lib/ejs/1.0.0/lib
     */
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, "."));
    relModDir = sfmt("%s/../%s", mprGetAppDir(ejs), BLD_LIB_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetAppDir(ejs)));
    relModDir = sfmt("%s/../%s", mprGetAppDir(ejs), BLD_LIB_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetAbsPath(relModDir)));
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, BLD_LIB_PREFIX));
#endif
    return (EjsArray*) ap;
}


int ejsEvalModule(cchar *path)
{
    Ejs             *ejs;
    Mpr             *mpr;
    int             status;

    status = 0;
    if ((mpr = mprCreate(0, NULL, 0)) == 0) {
        status = MPR_ERR_MEMORY;

    } else if ((ejs = ejsCreateVM(0, 0, 0)) == 0) {
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
    mprAssert(ejs->result == 0 || (MPR_GET_GEN(MPR_GET_MEM(ejs->result)) != MPR->heap->dead));

    if (className) {
        ejs->className = sclone(className);
    }
    if (methodName) {
        ejs->methodName = sclone(methodName);
    }
    mprRelayEvent(ejs->dispatcher, (MprEventProc) runProgram, ejs, NULL);

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
    if (className) {
        className = sclone(className);
    }
    if (methodName == 0) {
        methodName = "main";
    }
    methodName = sclone(methodName);
    /*  
        Search for the first class with the given name
     */
    if (className == 0 || *className == '\0') {
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


int ejsHasObservers(Ejs *ejs, EjsObj *emitter)
{
    if (emitter) {
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_hasObservers, 0, NULL);
    }
    return 0;
}


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
        av[1] = thisObj ? thisObj : ESV(null);
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
    globalCount = ejsGetLength(ejs, global);

    /*  
        Search for the named method in all types
     */
    for (slotNum = 0; slotNum < globalCount; slotNum++) {
        vp = ejsGetProperty(ejs, global, slotNum);
        if (vp == 0 || !ejsIsType(ejs, vp)) {
            continue;
        }
        type = (EjsType*) vp;
        methodCount = ejsGetLength(ejs, type);

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


#if UNUSED
static void logHandler(int flags, int level, cchar *msg)
{
    char        *prefix, *tag, *amsg, buf[MPR_MAX_STRING];
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
        tag = itos(level);
    }
    if (tag) {
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            /* Avoid allocation if possible */
            mprSprintf(buf, sizeof(buf), "%s: %s: %s\n", prefix, tag, msg);
            msg = buf;
        } else {
            msg = amsg = sfmt("%s: %s: %s\n", prefix, tag, msg);
        }
    }
    if (MPR->logFile) {
        mprFprintf(MPR->logFile, "%s", msg);
    } else {
        mprPrintfError("%s", msg);
    }
    solo = 0;
}
#endif


#if UNUSED
int ejsRedirectLogging(cchar *logSpec)
{
    MprFile     *file;
    char        *spec, *levelSpec;
    int         level;

    level = 0;
    if (logSpec == 0) {
        logSpec = "stderr:1";
    }
    spec = sclone(logSpec);

    if ((levelSpec = strchr(spec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }
    if (strcmp(spec, "stdout") == 0) {
        file = MPR->stdOutput;

    } else if (strcmp(spec, "stderr") == 0) {
        file = MPR->stdError;

    } else {
        //  TODO - should provide some means to append to the log
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
#endif


void ejsRedirectLoggingToFile(MprFile *file, int level)
{
    if (level >= 0) {
        mprSetLogLevel(level);
    }
    if (file) {
        mprSetLogFile(file);
    }
}


int ejsBlockGC(Ejs *ejs)
{
    int     paused;

    paused = ejs->state->paused;
    ejs->state->paused = 1;
    return paused;
}


void ejsUnblockGC(Ejs *ejs, int paused)
{
    mprAssert(paused != -1);
    if (paused != -1) {
        ejs->state->paused = paused;
    }
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
        msg = buf = sfmtv(fmt, arg);
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


void ejsLockService()
{
    mprLock(((EjsService*) MPR->ejsService)->mutex);
}


void ejsUnlockService()
{
    mprUnlock(((EjsService*) MPR->ejsService)->mutex);
}


void ejsLoadHttpService(Ejs *ejs)
{
    ejsLockService();
    if (mprGetMpr()->httpService == 0) {
        httpCreate();
    }
    ejs->http = ejs->service->http = mprGetMpr()->httpService;
    if (ejs->http == 0) {
        mprError("Can't load Http Service");
    }
    ejsUnlockService();
}


int ejsAddImmutable(Ejs *ejs, int slotNum, EjsName qname, EjsAny *value)
{
    int     foundSlot;

    mprAssert((ejsIsType(ejs, value) && !((EjsType*) value)->mutable) ||
              (!ejsIsType(ejs, value) && !TYPE(value)->mutableInstances));
    
    if ((foundSlot = ejsLookupProperty(ejs, ejs->service->immutable, qname)) >= 0) {
        return foundSlot;
    }
    slotNum = ejsSetProperty(ejs, ejs->service->immutable, slotNum, value);
    ejsSetPropertyName(ejs, ejs->service->immutable, slotNum, qname);
    return slotNum;
}


EjsAny *ejsGetImmutable(Ejs *ejs, int slotNum)
{
    return ejsGetProperty(ejs, ejs->service->immutable, slotNum);
}


EjsAny *ejsGetImmutableByName(Ejs *ejs, EjsName qname)
{
    return ejsGetPropertyByName(ejs, ejs->service->immutable, qname);
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
 
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
