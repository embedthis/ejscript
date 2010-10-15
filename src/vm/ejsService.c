/**
    ejsService.c - Ejscript interpreter factory

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forward **********************************/

static void allocFailure(Ejs *ejs, uint size, uint total, bool granted);
static int  configureEjs(Ejs *ejs);
static int  defineTypes(Ejs *ejs);
static int  destroyEjs(Ejs *ejs);
static int  destroyEjsService(EjsService *service);
static int  loadStandardModules(Ejs *ejs, MprList *require);
static void logHandler(MprCtx ctx, int flags, int level, cchar *msg);
static int  runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName);
static int  searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn);
static int  startLogging(Ejs *ejs);

/************************************* Code ***********************************/
/*  
    Initialize the ejs subsystem
 */
EjsService *ejsCreateService(MprCtx ctx)
{
    EjsService  *sp;

    if ((sp = mprAllocObj(ctx, EjsService, destroyEjsService)) == NULL) {
        return 0;
    }
    mprGetMpr(ctx)->ejsService = sp;
    sp->nativeModules = mprCreateHash(sp, -1, MPR_HASH_PERM_KEYS | MPR_HASH_UNICODE);
    sp->mutex = mprCreateLock(sp);
    return sp;
}


static int destroyEjsService(EjsService *service)
{
    return 0;
}


EjsService *ejsGetService(MprCtx ctx)
{
    return mprGetMpr(ctx)->ejsService;
}


/*  
    Create a new interpreter
    @param searchPath Array of paths to search for modules. Must be persistent.
    @param require List of modules to pre-load
    @param argc Count of command line args 
    @param argv Array of command line args
 */
Ejs *ejsCreateVm(MprCtx ctx, cchar *searchPath, MprList *require, int argc, cchar **argv, int flags)
{
    Ejs     *ejs;
    cchar   *name;
    static int seqno = 0;

    if ((ejs = mprAllocObj(ctx, Ejs, destroyEjs)) == NULL) {
        return 0;
    }
    mprSetAllocCallback(ejs, (MprAllocFailure) allocFailure);
    ejs->service = mprGetMpr(ctx)->ejsService;
    ejs->empty = require && mprGetListCount(require) == 0;
    ejs->heap = mprAllocCtx(ejs, 0);
    ejs->mutex = mprCreateLock(ejs);
    if (ejs->service->http == 0) {
        ejs->service->http = httpCreate(ejs->service);
    }
    ejs->http = ejs->service->http;
    ejs->argc = argc;
    ejs->argv = argv;

    ejs->flags |= (flags & (EJS_FLAG_NO_INIT | EJS_FLAG_DOC));
    name = mprAsprintf(ejs, -1, "ejsDispatcher-%d", seqno++);
    ejs->dispatcher = mprCreateDispatcher(ejs, name, 1);

    if ((ejs->bootSearch = searchPath) == 0) {
        ejs->bootSearch = getenv("EJSPATH");
    }
    if (ejsInitStack(ejs) < 0) {
        mprFree(ejs);
        return 0;
    }
    ejsCreateGCService(ejs);
    ejsCreateAllocService(ejs);
    ejsCreateStringService(ejs);
    ejsSetGeneration(ejs, EJS_GEN_ETERNAL);

    if (defineTypes(ejs) < 0 || loadStandardModules(ejs, require) < 0) {
        if (ejs->exception) {
            ejsReportError(ejs, "Can't initialize interpreter");
        }
        mprFree(ejs);
        return 0;
    }
    ejs->modules = ejsCreateArray(ejs, 0);
    ejs->workers = ejsCreateArray(ejs, 0);
#if UNUSED
    ejs->standardSpaces = mprCreateHash(ejs, 0);
#endif
    
    ejsFreezeGlobal(ejs);
    ejsMakeEternalPermanent(ejs);
    startLogging(ejs);

    if (mprHasAllocError(ejs)) {
        mprError(ejs, "Memory allocation error during initialization");
        mprFree(ejs);
        return 0;
    }
    ejsCollectGarbage(ejs, EJS_GEN_ETERNAL);
    ejsSetGeneration(ejs, EJS_GEN_NEW);
    return ejs;
}


static int destroyEjs(Ejs *ejs)
{
    EjsState    *state;

    ejsClearException(ejs);
    ejsDestroyGCService(ejs);
    state = ejs->masterState;
    if (state->stackBase) {
        mprVirtFree(state->stackBase, state->stackSize);
    }
    mprFree(ejs->heap);
    mprSetAltLogData(ejs, NULL);
    mprSetLogHandler(ejs, NULL, NULL);
    return 0;
}


void ejsCloneDefaultHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ejs->heap->type->helpers;
}


void ejsCloneObjectHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ejs->objectType->helpers;
}


void ejsCloneBlockHelpers(Ejs *ejs, EjsType *type)
{
    type->helpers = ejs->blockType->helpers;
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
    ejsBootstrapTypes(ejs);
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
#if UNUSED
    ejsCreateArrayType(ejs);
#endif
    ejsCreateBooleanType(ejs);
    ejsCreateErrorType(ejs);
    ejsCreateIteratorType(ejs);
    ejsCreateVoidType(ejs);
    ejsCreateNumberType(ejs);
    ejsCreateRegExpType(ejs);
#if UNUSED
    ejsCreateStringType(ejs);
#endif
    ejsCreateXMLType(ejs);
    ejsCreateXMLListType(ejs);
    ejsCreateConfigType(ejs);

    /*  
        Define the native module configuration routines.
     */
    ejsAddNativeModule(ejs, ejsCreateStringFromCS(ejs, "ejs"), configureEjs, _ES_CHECKSUM_ejs, 0);

#if BLD_FEATURE_EJS_ALL_IN_ONE || BLD_FEATURE_STATIC
#if BLD_FEATURE_SQLITE
    ejs_db_sqlite_Init(ejs);
#endif
    ejs_web_Init(ejs);
#endif
    
    if (ejs->hasError || mprHasAllocError(ejs)) {
        mprError(ejs, "Can't create core types");
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
    ejsConfigureDateType(ejs);
    ejsConfigureFunctionType(ejs);
    ejsConfigureGCType(ejs);
    ejsConfigureHttpType(ejs);
    ejsConfigureJSONType(ejs);
    ejsConfigureLoggerType(ejs);
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

    ejsInitSearchPath(ejs);
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
            if (strcmp(name, "ejs") == 0) {
                flags |= EJS_LOADER_BUILTIN;
            }
            rc += ejsLoadModule(ejs, ejsCreateStringFromCS(ejs, name), ver, ver, flags);
        }
    } else {
        rc += ejsLoadModule(ejs, ejsCreateStringFromCS(ejs, "ejs"), ver, ver, EJS_LOADER_STRICT | EJS_LOADER_BUILTIN);
    }
    return rc;
}


/*  
    Notifier callback function. Invoked by mprAlloc on allocation errors. This will prevent the allocation error
    bubbling up to the global memory failure handler.
 */
static void allocFailure(Ejs *ejs, uint size, uint total, bool granted)
{
    MprAllocStats   *alloc;
    EV              *argv[2], *thisObj;
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
            mprSprintf(ejs, msg, sizeof(msg), "Low memory condition. Total mem: %d. Request for %d bytes granted.", 
                total, size);
            ejsCreateException(ejs, ES_MemoryError, msg, dummy);
        }
    } else {
        if (!ejs->exception) {
            mprSprintf(ejs, msg, sizeof(msg), "Memory depleted. Total mem: %d. Request for %d bytes denied.", total, size);
            ejsCreateException(ejs, ES_MemoryError, msg, dummy);
        }
    }
}


void ejsInitSearchPath(Ejs *ejs)
{
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
    mprAssert(ejsIsArray(ejs, paths));

    ejs->search = paths;
}


EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *search)
{
    EV      *ap;
    char    *dir, *next, *tok;

    ap = ejsCreateArray(ejs, 0);

    if (search) {
        next = mprStrdup(ejs, search);
        dir = mprStrTok(next, MPR_SEARCH_SEP, &tok);
        while (dir && *dir) {
            ejsSetProperty(ejs, ap, -1, ejsCreatePathFromCS(ejs, dir));
            dir = mprStrTok(NULL, MPR_SEARCH_SEP, &tok);
        }
        mprFree(next);
        return (EjsArray*) ap;
    }
#if VXWORKS
    ejsSetProperty(ejs, ap, -1, ejsCreatePathAndFree(ejs, mprGetCurrentPath(ejs)));
#else
    /*
        Create a default search path
        "." : APP_EXE_DIR/../modules : /usr/lib/ejs/1.0.0/modules
     */
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromCS(ejs, "."));
    char *relModDir;
    relModDir = mprAsprintf(ejs, -1, "%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathAndFree(ejs, mprGetAppDir(ejs)));
#ifdef BLD_MOD_NAME
{
    char *relModDir;
    relModDir = mprAsprintf(ejs, -1, "%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathAndFree(ejs, mprGetAbsPath(ejs, relModDir)));
    mprFree(relModDir);
}
#endif
#ifdef BLD_MOD_PREFIX
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromCS(ejs, BLD_MOD_PREFIX));
#endif
#endif
    return (EjsArray*) ap;
}


EV *ejsGetGlobalObject(Ejs *ejs)
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
    EjsService      *service;   
    Ejs             *ejs;
    Mpr             *mpr;

    mpr = mprCreate(0, NULL, NULL);
    if ((service = ejsCreateService(mpr)) == 0) {
        mprFree(mpr);
        return MPR_ERR_NO_MEMORY;
    }
    if ((ejs = ejsCreateVm(service, NULL, NULL, 0, NULL, 0)) == 0) {
        mprFree(mpr);
        return MPR_ERR_NO_MEMORY;
    }
    if (ejsLoadModule(ejs, ejsCreateStringFromCS(ejs, path), -1, -1, 0) < 0) {
        mprFree(mpr);
        return MPR_ERR_CANT_READ;
    }
    if (ejsRun(ejs) < 0) {
        mprFree(mpr);
        return EJS_ERR;
    }
    mprFree(mpr);
    return 0;
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
    ejs->className = className;
    ejs->methodName = methodName;
    mprRelayEvent(ejs->dispatcher, (MprEventProc) runProgram, ejs, NULL);

    if (ejs->flags & EJS_FLAG_NOEXIT) {
        /*
            If the script calls App.noexit(), this will service events until App.exit() is called.
            TODO - should deprecate noexit()
         */
        mprServiceEvents(ejs, ejs->dispatcher, -1, 0);
    }
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
    EV              *args;
    int             attributes, i, slotNum;

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
        mprError(ejs, "Can't find class \"%s\"", className);
        return EJS_ERR;
    }
    slotNum = ejsLookupProperty(ejs, type, N(EJS_PUBLIC_NAMESPACE, methodName));
    if (slotNum < 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    fun = (EjsFunction*) ejsGetProperty(ejs, type, slotNum);
    if (! ejsIsFunction(ejs, fun)) {
        mprError(ejs, "Property \"%s\" is not a function");
        return MPR_ERR_BAD_STATE;
    }
    attributes = ejsGetTypePropertyAttributes(ejs, type, slotNum);
    if (!(attributes & EJS_PROP_STATIC)) {
        mprError(ejs, "Method \"%s\" is not declared static");
        return EJS_ERR;
    }
    args = ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, ejsCreateStringFromCS(ejs, ejs->argv[i]));
    }
    if (ejsRunFunction(ejs, fun, 0, 1, &args) == 0) {
        return EJS_ERR;
    }
    return 0;
}


int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener)
{
    EV          *emitter, *argv[2];
    EjsArray    *list;
    int         i;

    if (*emitterPtr == 0) {
        *emitterPtr = ejsCreateInstance(ejs, ejsGetTypeByName(ejs, EJS_EJS_NAMESPACE, "Emitter"), 0, NULL);
    }
    emitter = *emitterPtr;

    argv[1] = listener;
    if (ejsIsArray(ejs, name)) {
        list = (EjsArray*) name;
        for (i = 0; i < list->length; i++) {
            name = ejsGetProperty(ejs, list, i);
            if (!ejsIsNull(ejs, name)) {
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


int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener)
{
    EV          *argv[2];
    EjsArray    *list;
    int         i;

    if (emitter) {
        argv[1] = listener;
        if (ejsIsArray(ejs, name)) {
            list = (EjsArray*) name;
            for (i = 0; i < list->length; i++) {
                name = ejsGetProperty(ejs, list, i);
                if (!ejsIsNull(ejs, name)) {
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


int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EV *thisObj, int argc, EV **argv)
{
    EV      **av;
    int     i;

    if (emitter) {
        av = (EV**) mprAlloc(emitter, (argc + 2) * sizeof(EV*));
        av[0] = ejsCreateStringFromCS(ejs, name);
        av[1] = thisObj ? thisObj : ejs->nullValue;
        for (i = 0; i < argc; i++) {
            av[i + 2] = argv[i];
        }
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_fireThis, argc + 2, av);
        mprFree(av);
    }
    return 0;
}


int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EV *thisObj, EV *arg)
{
    EV      **argv;

    argv = &arg;
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
    EV          *global, *vp;
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
            if (qname.name && qname.name == ejsCreateStringFromCS(ejs, methodName)) {
                *typeReturn = type;
            }
        }
    }
    return 0;
}


typedef struct EjsLogData {
    Ejs         *ejs;
    EjsObj      *log;
    EjsFunction *loggerWrite;
    int         writeSlot;
} EjsLogData;


static int startLogging(Ejs *ejs)
{
    EjsLogData  *ld;
    EjsObj      *app;

    if ((ld = mprAllocObj(ejs, EjsLogData, NULL))  == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    ld->ejs = ejs;

    if ((app = ejsGetPropertyByName(ejs, ejs->global, N(EJS_EJS_NAMESPACE, "App"))) == 0) {
        return MPR_ERR_CANT_READ;
    }
    if ((ld->log = ejsGetPropertyByName(ejs, app, N(EJS_PUBLIC_NAMESPACE, "log"))) == 0) {
        return MPR_ERR_CANT_READ;
    }
    if ((ld->loggerWrite = ejsGetPropertyByName(ejs, TYPE(ld->log)->prototype, N(EJS_PUBLIC_NAMESPACE, "write"))) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mprSetAltLogData(ejs, ld);
    return 0;
}


static void logHandler(MprCtx ctx, int flags, int level, cchar *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    Ejs         *ejs;
    EjsLogData  *ld;
    EV          *str, *saveException;
    static int  solo = 0;
    char        *prefix, *tag, *amsg, lbuf[16], buf[MPR_MAX_STRING];

    //  MOB - not thread safe
    if (solo > 0) {
        return;
    }
    solo++;
    mpr = mprGetMpr(ctx);
    prefix = mpr->name;
    amsg = NULL;

    if (flags & MPR_ERROR_SRC) {
        tag = "Error";
    } else if (flags & MPR_FATAL_SRC) {
        tag = "Fatal";
    } else if (flags & MPR_RAW) {
        tag = NULL;
    } else {
        tag = mprItoa(lbuf, sizeof(lbuf), level, 10);
    }
    if (tag) {
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            /* Avoid allocation if possible */
            mprSprintf(ctx, buf, sizeof(buf), "%s: %s: %s\n", prefix, tag, msg);
            msg = buf;
        } else {
            msg = amsg = mprAsprintf(ctx, -1, "%s: %s: %s\n", prefix, tag, msg);
        }
    }
    ld = (EjsLogData*) mpr->altLogData;
    if (ld && ld->loggerWrite) {
        ejs = ld->ejs;
        str = (EjsObj*) ejsCreateStringFromCS(ejs, msg);
        saveException = ejs->exception;
        ejsClearException(ejs);
        ejsRunFunction(ejs, ld->loggerWrite, ld->log, 1, &str);
        ejs->exception = saveException;

    } else if (mpr->logData) {
        file = (MprFile*) mpr->logData;
        mprFprintf(file, "%s", msg);

    } else {
        file = (MprFile*) mpr->logData;
        mprPrintfError(ctx, "%s", msg);
    }
    mprFree(amsg);
    solo--;
}


int ejsStartMprLogging(Mpr *mpr, char *logSpec)
{
    MprFile     *file;
    char        *levelSpec;
    int         level;

    level = 0;
    logSpec = mprStrdup(mpr, logSpec);

    if ((levelSpec = strchr(logSpec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }
    if (strcmp(logSpec, "stdout") == 0) {
        file = mpr->fileSystem->stdOutput;

    } else if (strcmp(logSpec, "stderr") == 0) {
        file = mpr->fileSystem->stdError;

    } else {
        if ((file = mprOpen(mpr, logSpec, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
            mprPrintfError(mpr, "Can't open log file %s\n", logSpec);
            mprFree(logSpec);
            return EJS_ERR;
        }
    }
    mprSetLogFd(mpr, mprGetFileFd(file));
    mprSetLogLevel(mpr, level);
    mprSetLogHandler(mpr, logHandler, (void*) file);
    mprFree(logSpec);
    return 0;
}


/*  
    Global memory allocation handler. This is invoked when there is no notifier to handle an allocation failure.
    The interpreter has an allocFailure (see ejsService: allocFailure) and it will handle allocation errors.
 */
void ejsMemoryFailure(MprCtx ctx, size_t size, size_t total, bool granted)
{
    if (!granted) {
        mprPrintfError(ctx, "Can't allocate memory block of size %d\n", size);
        mprPrintfError(ctx, "Total memory used %d\n", total);
        exit(255);
    }
    mprPrintfError(ctx, "Memory request for %d bytes exceeds memory red-line\n", size);
    mprPrintfError(ctx, "Total memory used %d\n", total);
}


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
        msg = buf = mprVasprintf(ejs, 0, fmt, arg);
    }
    if (ejs->exception) {
        mprRawLog(ejs, 0, "%s: %s\n", mprGetMpr(ejs)->name, msg);
    } else {
        mprError(ejs, "%s", msg);
    }
    mprFree(buf);
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
