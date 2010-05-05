/**
    ejsService.c - Ejscript interpreter factory

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forward **********************************/

static void allocFailure(Ejs *ejs, uint size, uint total, bool granted);
static int  cloneMaster(Ejs *ejs, Ejs *master);
static int  configureEjs(Ejs *ejs);
static int  defineTypes(Ejs *ejs);
static int  destroyEjs(Ejs *ejs);
static int  loadStandardModules(Ejs *ejs, MprList *require);
static int  runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName);
static int  searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn);

/************************************* Code ***********************************/
/*  
    Initialize the EJS subsystem
 */
EjsService *ejsCreateService(MprCtx ctx)
{
    EjsService  *sp;

    sp = mprAllocObjZeroed(ctx, EjsService);
    if (sp == 0) {
        return 0;
    }
    mprGetMpr(ctx)->ejsService = sp;
    sp->nativeModules = mprCreateHash(sp, -1);
    return sp;
}


EjsService *ejsGetService(MprCtx ctx)
{
    return mprGetMpr(ctx)->ejsService;
}


/*  
    Create a new interpreter
    @param master Master interpreter to clone for rapid-startup
    @param searchPath Array of paths to search for modules. Must be persistent.
    @param require List of modules to pre-load
 */
Ejs *ejsCreateVm(MprCtx ctx, Ejs *master, cchar *searchPath, MprList *require, int flags)
{
    Ejs     *ejs;

    /*  
        Create interpreter structure
     */
    ejs = mprAllocObjWithDestructorZeroed(ctx, Ejs, destroyEjs);
    if (ejs == 0) {
        return 0;
    }
    mprSetAllocCallback(ejs, (MprAllocFailure) allocFailure);
    ejs->service = mprGetMpr(ctx)->ejsService;
    ejs->empty = require && mprGetListCount(require) == 0;
    ejs->heap = mprAllocHeap(ejs, "Ejs Object Heap", 1, 0, NULL);
    ejs->mutex = mprCreateLock(ejs);
    if (ejs->service->http == 0) {
        ejs->service->http = httpCreate(ejs->service);
    }
    ejs->http = ejs->service->http;

    ejs->flags |= (flags & (EJS_FLAG_NO_INIT | EJS_FLAG_DOC));
    ejs->dispatcher = mprCreateDispatcher(ejs, "ejsDispatcher", 1);

    if ((ejs->bootSearch = searchPath) == 0) {
        ejs->bootSearch = getenv("EJSPATH");
    }
    if (ejsInitStack(ejs) < 0) {
        mprFree(ejs);
        return 0;
    }
    ejsCreateGCService(ejs);
    ejsSetGeneration(ejs, EJS_GEN_ETERNAL);

    if (master == 0) {
        ejs->modules = mprCreateList(ejs);
        ejs->workers = mprCreateList(ejs);
        ejs->coreTypes = mprCreateHash(ejs, 0);
        ejs->standardSpaces = mprCreateHash(ejs, 0);
        
        if (defineTypes(ejs) < 0 || loadStandardModules(ejs, require) < 0) {
            if (ejs->exception) {
                ejsReportError(ejs, "Can't initialize interpreter");
            }
            mprFree(ejs);
            return 0;
        }
    } else {
        cloneMaster(ejs, master);
    }
    ejsFreezeGlobal(ejs);
    ejsMakeEternalPermanent(ejs);

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

    ejsDestroyGCService(ejs);
    state = ejs->masterState;
    if (state->stackBase) {
        mprMapFree(state->stackBase, state->stackSize);
    }
    mprFree(ejs->heap);
	return 0;
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
    ejsCreateArrayType(ejs);
    ejsCreateBooleanType(ejs);
    ejsCreateErrorType(ejs);
    ejsCreateIteratorType(ejs);
    ejsCreateVoidType(ejs);
    ejsCreateNumberType(ejs);
    ejsCreateRegExpType(ejs);
    ejsCreateStringType(ejs);
    ejsCreateXMLType(ejs);
    ejsCreateXMLListType(ejs);

    /*  
        Define the native module configuration routines.
     */
    ejsAddNativeModule(ejs->service, "ejs", configureEjs, _ES_CHECKSUM_ejs, 0);

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
    ejsConfigureConfigType(ejs);
    ejsConfigureDateType(ejs);
    ejsConfigureEventType(ejs);
    ejsConfigureFunctionType(ejs);
    ejsConfigureGCType(ejs);
    ejsConfigureHttpType(ejs);
    ejsConfigureJSONType(ejs);
    ejsConfigureLoggerType(ejs);
    ejsConfigureMathType(ejs);
    ejsConfigureMemoryType(ejs);
    ejsConfigureNamespaceType(ejs);
    ejsConfigureReflectType(ejs);
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
    int     rc, next, ver;

    rc = 0;

    ver = 0;
    if (require) {
        for (next = 0; rc == 0 && (name = mprGetNextItem(require, &next)) != 0; ) {
            rc += ejsLoadModule(ejs, name, ver, ver, EJS_LOADER_STRICT);
        }
    } else {
        rc += ejsLoadModule(ejs, "ejs", ver, ver, EJS_LOADER_STRICT);
    }
    return rc;
}


/* 
   When cloning the master interpreter, the new interpreter references the master's core types. The core types MUST
    be immutable for this to work.
 */
static int cloneMaster(Ejs *ejs, Ejs *master)
{
    EjsName     qname;
    EjsObj      *vp;
    EjsTrait    *trait;
    int         i, count;

    mprAssert(master);

    ejs->master = master;
    ejs->service = master->service;

    //  TODO - OPT

    ejs->objectType = master->objectType;
    ejs->arrayType = master->arrayType;
    ejs->blockType = master->blockType;
    ejs->booleanType = master->booleanType;
    ejs->byteArrayType = master->byteArrayType;
    ejs->dateType = master->dateType;
    ejs->errorType = master->errorType;
    ejs->eventType = master->eventType;
    ejs->errorEventType = master->errorEventType;
    ejs->functionType = master->functionType;
    ejs->iteratorType = master->iteratorType;
    ejs->namespaceType = master->namespaceType;
    ejs->nullType = master->nullType;
    ejs->numberType = master->numberType;
    ejs->objectType = master->objectType;
    ejs->pathType = master->pathType;
    ejs->requestType = master->requestType;
    ejs->regExpType = master->regExpType;
    ejs->stringType = master->stringType;
    ejs->stopIterationType = master->stopIterationType;
    ejs->typeType = master->typeType;
    ejs->voidType = master->voidType;
    ejs->webType = master->webType;
    ejs->workerType = master->workerType;

    ejs->emptyStringValue = master->emptyStringValue;
    ejs->falseValue = master->falseValue;
    ejs->infinityValue = master->infinityValue;
    ejs->minusOneValue = master->minusOneValue;
    ejs->nanValue = master->nanValue;
    ejs->negativeInfinityValue = master->negativeInfinityValue;
    ejs->nullValue = master->nullValue;
    ejs->oneValue = master->oneValue;
    ejs->trueValue = master->trueValue;
    ejs->undefinedValue = master->undefinedValue;
    ejs->zeroValue = master->zeroValue;

    ejs->emptySpace = master->emptySpace;
    ejs->ejsSpace = master->ejsSpace;
    ejs->iteratorSpace = master->iteratorSpace;
    ejs->internalSpace = master->internalSpace;
    ejs->publicSpace = master->publicSpace;

    ejs->argv = master->argv;
    ejs->argc = master->argc;
    ejs->coreTypes = master->coreTypes;
    ejs->standardSpaces = master->standardSpaces;

    ejs->modules = mprDupList(ejs, master->modules);

    ejs->xmlType = master->xmlType;
    ejs->xmlListType = master->xmlListType;

    ejs->sqlite = master->sqlite;

    ejs->globalBlock = ejsCreateBlock(ejs, master->globalBlock->obj.sizeSlots);
    ejs->global = (EjsObj*) ejs->globalBlock; 
    ejsSetDebugName(ejs->global, "global");
    ejs->global->numSlots = master->global->numSlots;

    //  TODO what about traits
    ejs->global->numSlots = master->global->numSlots;

    ejsCopyList(ejs->globalBlock, &ejs->globalBlock->namespaces, &master->globalBlock->namespaces);

    /*
        Copy global properties. If the type is immutable (!dynamic) then just copy the reference, else clone.
     */ 
    count = ejsGetPropertyCount(master, master->global);
    for (i = 0; i < count; i++) {
        vp = ejsGetProperty(ejs, master->global, i);
        if (vp) {
            if (ejsIsType(vp) && !vp->dynamic) {
                ejsSetProperty(ejs, ejs->global, i, ejsGetProperty(master, master->global, i));
            } else {
                ejsSetProperty(ejs, ejs->global, i, ejsClone(ejs, vp, 1));
            }
            qname = ejsGetPropertyName(master, master->global, i);
            ejsSetPropertyName(ejs, ejs->global, i, &qname);
            trait = ejsGetTrait(master->global, i);
            if (trait) {
                ejsSetPropertyTrait(ejs, ejs->global, i, trait->type, trait->attributes);
            }
        }
    }
    ejsSetProperty(ejs, ejs->global, ES_global, ejs->global);
    ejs->initialized = 1;
    return 0;
}


/*  
    Notifier callback function. Invoked by mprAlloc on allocation errors. This will prevent the allocation error
    bubbling up to the global memory failure handler.
 */
static void allocFailure(Ejs *ejs, uint size, uint total, bool granted)
{
    MprAlloc    *alloc;
    EjsObj      *argv[2], *thisObj;
    char        msg[MPR_MAX_STRING];
    va_list     dummy = NULL_INIT;

    alloc = mprGetAllocStats(ejs);
    if (granted) {
        if (ejs->memoryCallback) {
            argv[0] = (EjsObj*) ejsCreateNumber(ejs, size);
            argv[1] = (EjsObj*) ejsCreateNumber(ejs, total);
            thisObj = ejs->memoryCallback->thisObj ? ejs->memoryCallback->thisObj : ejs->global; 
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


void ejsInitSearchPath(Ejs *ejs)
{
    if (ejs->bootSearch) {
        ejs->search = ejsCreateSearchPath(ejs, ejs->bootSearch);
    } else {
        ejs->search = ejsCreateSearchPath(ejs, NULL);
    }
}

/*
    Set the module search path
 */
void ejsSetSearchPath(Ejs *ejs, EjsArray *paths)
{
    mprAssert(ejs);
    mprAssert(paths && paths);
    mprAssert(ejsIsArray(paths));

    ejs->search = paths;
}


EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *search)
{
    EjsObj      *ap;
    char        *dir, *next, *tok;

    ap = (EjsObj*) ejsCreateArray(ejs, 0);

    if (search) {
        next = mprStrdup(ejs, search);
        dir = mprStrTok(next, MPR_SEARCH_SEP, &tok);
        while (dir && *dir) {
            ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePath(ejs, dir));
            dir = mprStrTok(NULL, MPR_SEARCH_SEP, &tok);
        }
        mprFree(next);
        return (EjsArray*) ap;
    }
    /*
        Create a default search path
        "." : APP_DIR : APP_EXE_DIR/../modules : /usr/lib/ejs/1.0.0/modules
     */
#if VXWORKS
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePathAndFree(ejs, mprGetCurrentPath(ejs)));
#elif WIN
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePathAndFree(ejs, mprGetAppDir(ejs)));
#else
{
    char *relModDir;
    relModDir = mprAsprintf(ejs, -1, "%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePath(ejs, "."));
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePath(ejs, mprGetAppDir(ejs)));
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePathAndFree(ejs, mprGetAbsPath(ejs, relModDir)));
    ejsSetProperty(ejs, ap, -1, (EjsObj*) ejsCreatePath(ejs, BLD_MOD_PREFIX));
}
#endif
    return (EjsArray*) ap;
}


EjsObj *ejsGetGlobalObject(Ejs *ejs)
{
    return (EjsObj*) ejs->global;
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


/*  
    Evaluate a module file
 */
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
    if ((ejs = ejsCreateVm(service, NULL, NULL, NULL, 0)) == 0) {
        mprFree(mpr);
        return MPR_ERR_NO_MEMORY;
    }
    if (ejsLoadModule(ejs, path, -1, -1, 0) < 0) {
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


/*  
    Run a program. This is called from the dispatcher event loop.
 */
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
        return -1;
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
    EjsName         qname;
    EjsObj          *args;
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
        ejsName(&qname, EJS_PUBLIC_NAMESPACE, className);
        type = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, &qname);
    }

    if (type == 0 || !ejsIsType(type)) {
        mprError(ejs, "Can't find class \"%s\"", className);
        return EJS_ERR;
    }

    ejsName(&qname, EJS_PUBLIC_NAMESPACE, methodName);
    slotNum = ejsLookupProperty(ejs, (EjsObj*) type, &qname);
    if (slotNum < 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    fun = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) type, slotNum);
    if (! ejsIsFunction(fun)) {
        mprError(ejs, "Property \"%s\" is not a function");
        return MPR_ERR_BAD_STATE;
    }

    attributes = ejsGetTypePropertyAttributes(ejs, (EjsObj*) type, slotNum);
    if (!(attributes & EJS_PROP_STATIC)) {
        mprError(ejs, "Method \"%s\" is not declared static");
        return EJS_ERR;
    }
    args = (EjsObj*) ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, (EjsObj*) ejsCreateString(ejs, ejs->argv[i]));
    }
    if (ejsRunFunction(ejs, fun, 0, 1, &args) == 0) {
        return EJS_ERR;
    }
    return 0;
}


/*  
    Add the listener function. The event emitter is created on-demand.
 */
int ejsAddListener(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener)
{
    EjsObj      *emitter, *argv[2];
    EjsArray    *list;
    int         i;

    if (*emitterPtr == 0) {
        *emitterPtr = ejsCreateInstance(ejs, ejsGetTypeByName(ejs, EJS_EJS_NAMESPACE, "Emitter"), 0, NULL);
    }
    emitter = *emitterPtr;

    argv[1] = listener;
    if (ejsIsArray(name)) {
        list = (EjsArray*) name;
        for (i = 0; i < list->length; i++) {
            name = ejsGetProperty(ejs, (EjsObj*) list, i);
            if (!ejsIsNull(name)) {
                argv[0] = name;
                ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_addListener, 2, argv);
            }
        }
    } else {
        argv[0] = name;
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_addListener, 2, argv);
    }
    return 0;
}


#if ES_Emitter_hasListeners
int ejsHasListeners(Ejs *ejs, EjsObj *emitter)
{
    if (emitter) {
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_hasListeners, 0, NULL);
    }
    return 0;
}
#endif


int ejsRemoveListener(Ejs *ejs, EjsObj *emitter, EjsObj *name, EjsObj *listener)
{
    EjsObj      *argv[2];
    EjsArray    *list;
    int         i;

    if (emitter) {
        argv[1] = listener;
        if (ejsIsArray(name)) {
            list = (EjsArray*) name;
            for (i = 0; i < list->length; i++) {
                name = ejsGetProperty(ejs, (EjsObj*) list, i);
                if (!ejsIsNull(name)) {
                    argv[0] = name;
                    ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_removeListener, 2, argv);
                }
            }
        } else {
            argv[0] = name;
            ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_removeListener, 2, argv);
        }
    }
    return 0;
}


int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, int argc, EjsObj **argv)
{
    EjsObj      **av;
    int         i;

    if (emitter) {
        av = (EjsObj**) mprAlloc(emitter, (argc + 1) * sizeof(EjsObj*));
        av[0] = (EjsObj*) ejsCreateString(ejs, name);
        for (i = 0; i < argc; i++) {
            av[i + 1] = argv[i];
        }
        //  TODO - rename fire or send()
        ejsRunFunctionBySlot(ejs, emitter, ES_Emitter_emit, argc + 1, av);
        mprFree(av);
    }
    return 0;
}


int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsObj *arg)
{
    EjsObj      **argv;

    argv = &arg;
    return ejsSendEventv(ejs, emitter, name, 1, argv);
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
        if (vp == 0 || !ejsIsType(vp)) {
            continue;
        }
        type = (EjsType*) vp;

        methodCount = ejsGetPropertyCount(ejs, (EjsObj*) type);

        for (methodSlot = 0; methodSlot < methodCount; methodSlot++) {
            method = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) type, methodSlot);
            if (method == 0) {
                continue;
            }

            qname = ejsGetPropertyName(ejs, (EjsObj*) type, methodSlot);
            if (qname.name && strcmp(qname.name, methodName) == 0) {
                *typeReturn = type;
            }
        }
    }
    return 0;
}


static void logHandler(MprCtx ctx, int flags, int level, const char *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *prefix;

    mpr = mprGetMpr(ctx);
    file = (MprFile*) mpr->logHandlerData;
    prefix = mpr->name;

    while (*msg == '\n') {
        mprFprintf(file, "\n");
        msg++;
    }
    if (flags & MPR_LOG_SRC) {
        mprFprintf(file, "%s: %d: %s\n", prefix, level, msg);

    } else if (flags & MPR_ERROR_SRC) {
        /*  
            Use static printing to avoid malloc when the messages are small.
            This is important for memory allocation errors.
         */
        if (strlen(msg) < (MPR_MAX_STRING - 32)) {
            mprStaticPrintfError(file, "%s: Error: %s\n", prefix, msg);
        } else {
            mprFprintf(file, "%s: Error: %s\n", prefix, msg);
        }
    } else if (flags & MPR_FATAL_SRC) {
        mprFprintf(file, "%s: Fatal: %s\n", prefix, msg);
    } else if (flags & MPR_RAW) {
        mprFprintf(file, "%s", msg);
    }
}


int ejsStartLogging(Mpr *mpr, char *logSpec)
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
void ejsMemoryFailure(MprCtx ctx, int64 size, int64 total, bool granted)
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
    const char  *msg;
    char        *buf;

    va_start(arg, fmt);
    
    /*  
        Compiler error format is:
            program:line:errorCode:SEVERITY: message
        Where program is either "ec" or "ejs"
        Where SEVERITY is either "error" or "warn"
     */
    buf = mprVasprintf(ejs, 0, fmt, arg);
    msg = ejsGetErrorMsg(ejs, 1);
    
    mprError(ejs, "%s", (msg && *msg) ? msg: buf);
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
