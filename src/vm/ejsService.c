/**
    ejsService.c - Ejscript interpreter factory

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forward **********************************/

static int allocNotifier(int flags, ssize size);
static int  configureEjs(Ejs *ejs);
static int  defineTypes(Ejs *ejs);
static void manageEjs(Ejs *ejs, int flags);
static void manageEjsService(EjsService *service, int flags);
static void markValues(Ejs *ejs);
static int  loadStandardModules(Ejs *ejs, MprList *require);
static void logHandler(int flags, int level, cchar *msg);
static int  runSpecificMethod(Ejs *ejs, cchar *className, cchar *methodName);
static int  searchForMethod(Ejs *ejs, cchar *methodName, EjsType **typeReturn);

/************************************* Code ***********************************/
/*  
    Initialize the ejs subsystem
 */
EjsService *ejsCreateService()
{
    EjsService  *sp;

    if ((sp = mprAllocObj(EjsService, manageEjsService)) == NULL) {
        return 0;
    }
    mprGetMpr()->ejsService = sp;
    mprSetMemNotifier((MprMemNotifier) allocNotifier);

    sp->nativeModules = mprCreateHash(-1, MPR_HASH_PERM_KEYS | MPR_HASH_UNICODE);
    sp->mutex = mprCreateLock();
    sp->vmlist = mprCreateList();
    return sp;
}


static void manageEjsService(EjsService *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->http);
        mprMark(sp->mutex);
        mprMark(sp->vmlist);
        mprLock(sp->mutex);
        mprMarkHash(sp->nativeModules);
        mprUnlock(sp->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        mprRemoveRoot(sp);
        sp->mutex = NULL;
    }
}


EjsService *ejsGetService()
{
    return mprGetMpr()->ejsService;
}


/*  
    Create a new interpreter
    @param searchPath Array of paths to search for modules. Must be persistent.
    @param require List of modules to pre-load
    @param argc Count of command line args 
    @param argv Array of command line args
 */
Ejs *ejsCreateVm(cchar *searchPath, MprList *require, int argc, cchar **argv, int flags)
{
    EjsService  *sp;
    Ejs         *ejs;
    int         save;
    static int  seqno = 0;

    save = mprEnableGC(0);
    if ((ejs = mprAllocObj(Ejs, manageEjs)) == NULL) {
        mprEnableGC(save);
        return 0;
    }
    sp = ejs->service = mprGetMpr()->ejsService;
    mprLock(sp->mutex);
    mprAddItem(sp->vmlist, ejs);
    mprUnlock(sp->mutex);

    ejs->empty = require && mprGetListLength(require) == 0;
    ejs->mutex = mprCreateLock(ejs);
    ejs->argc = argc;
    ejs->argv = argv;

    ejs->flags |= (flags & (EJS_FLAG_NO_INIT | EJS_FLAG_DOC));
    ejs->dispatcher = mprCreateDispatcher(mprAsprintf("ejsDispatcher-%d", seqno++), 1);
    ejs->modules = mprCreateList();
    ejs->workers = mprCreateList();
        
    if ((ejs->bootSearch = searchPath) == 0) {
        ejs->bootSearch = getenv("EJSPATH");
    }
    if (ejsInitStack(ejs) < 0) {
        ejsDestroy(ejs);
        mprEnableGC(save);
        return 0;
    }
    if (defineTypes(ejs) < 0 || loadStandardModules(ejs, require) < 0) {
        if (ejs->exception) {
            ejsReportError(ejs, "Can't initialize interpreter");
        }
        ejsDestroy(ejs);
        mprEnableGC(save);
        return 0;
    }
    ejsFreezeGlobal(ejs);
#if UNUSED && KEEP
    startLogging(ejs);
#endif
    if (mprHasMemError(ejs)) {
        mprError("Memory allocation error during initialization");
        ejsDestroy(ejs);
        mprEnableGC(save);
        return 0;
    }
    mprEnableGC(save);
    return ejs;
}


void ejsDestroy(Ejs *ejs)
{
    EjsService  *sp;
    EjsState    *state;

    sp = ejs->service;
    if (sp) {
        ejsDestroyIntern(ejs);
        state = ejs->masterState;
        if (state->stackBase) {
            mprVirtFree(state->stackBase, state->stackSize);
            state->stackBase = 0;
            ejs->masterState = 0;
        }
        lock(sp);
        mprRemoveItem(sp->vmlist, ejs);
        unlock(sp);
        ejs->service = 0;
    }
}


static void manageEjs(Ejs *ejs, int flags)
{
    EjsBlock    *block;
    EjsObj      *vp, **vpp, **top;

    if (flags & MPR_MANAGE_MARK) {
        mprMarkList(ejs->modules);
        mprMark(ejs->applications);
        mprMark(ejs->coreTypes);
        mprMark(ejs->doc);
        mprMark(ejs->errorMsg);
        mprMark(ejs->exception);
        mprMark(ejs->exceptionArg);
        mprMark(ejs->masterState);
        mprMark(ejs->mutex);
        mprMark(ejs->result);
        mprMark(ejs->search);
        mprMark(ejs->dispatcher);
        mprMarkList(ejs->workers);
        mprMark(ejs->global);

        /*
            Mark active call stack
         */
        for (block = ejs->state->bp; block; block = block->prev) {
            mprMark(block);
        }
        /*
            Mark the evaluation stack. Stack itself is virtually allocated and immune from GC.
         */
        top = ejs->state->stack;
        for (vpp = ejs->state->stackBase; vpp <= top; vpp++) {
            if ((vp = *vpp) != NULL) {
                mprMark(vp);
            }
        }
        markValues(ejs);
        ejsManageIntern(ejs, flags);

    } else if (flags & MPR_MANAGE_FREE) {
        ejsDestroy(ejs);
    }
}


#if UNUSED
static void markGlobal(Ejs *ejs)
{
    EjsPot      *obj;
    EjsBlock    *block;
    int         i, numProp;

    obj = (EjsPot*) ejs->global;
    numProp = ejsGetPropertyCount(ejs, obj);

    //  MOB - fix making core eternal
    for (i = ejs->firstGlobal; i < numProp; i++) {
        mprMark(obj->properties->slots[i].value.ref);
    }
    block = (EjsBlock*) ejs->global;
    if (block->prevException) {
        mprMark(block->prevException);
    }
    if (block->namespaces.length > 0) {
        for (i = 0; i < block->namespaces.length; i++) {
            //  MOB - OPT
            mprMark(mprGetItem(&block->namespaces, i));
        }
    }
}
#endif


static void markValues(Ejs *ejs)
{
    int     i;

    for (i = 0; i < EJS_MAX_SPECIAL; i++) {
        mprMark(ejs->values[i]);
    }
    mprMark(ejs->commaProtString);
    mprMark(ejs->ejsSpace);
    mprMark(ejs->emptySpace);
    mprMark(ejs->emptyString);
    mprMark(ejs->falseValue);
    mprMark(ejs->infinityValue);
    mprMark(ejs->internalSpace);
    mprMark(ejs->iterator);
    mprMark(ejs->iteratorSpace);
    mprMark(ejs->lengthString);
    mprMark(ejs->maxValue);
    mprMark(ejs->minValue);
    mprMark(ejs->minusOneValue);
    mprMark(ejs->nanValue);
    mprMark(ejs->negativeInfinityValue);
    mprMark(ejs->nopFunction);
    mprMark(ejs->nullValue);
    mprMark(ejs->oneValue);
    mprMark(ejs->publicSpace);
    mprMark(ejs->publicString);
    mprMark(ejs->trueValue);
    mprMark(ejs->undefinedValue);
    mprMark(ejs->zeroValue);
    mprMark(ejs->nopFunction);
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
    type->helpers = ejs->blockType->helpers;
    type->isPot = 1;
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
    ejsCreateRegExpType(ejs);
    ejsCreateXMLType(ejs);
    ejsCreateXMLListType(ejs);
    ejsCreateConfigType(ejs);

    /*  
        Define the native module configuration routines.
     */
    ejsAddNativeModule(ejs, ejsCreateStringFromAsc(ejs, "ejs"), configureEjs, _ES_CHECKSUM_ejs, 0);

#if BLD_FEATURE_EJS_ALL_IN_ONE || BLD_STATIC
#if BLD_FEATURE_SQLITE
    ejs_db_sqlite_Init(ejs);
#endif
    ejs_web_Init(ejs);
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
#if UNUSED
            if (strcmp(name, "ejs") == 0) {
                flags |= EJS_LOADER_BUILTIN;
            }
#endif
            rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, name), ver, ver, flags);
        }
    } else {
        rc += ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, "ejs"), ver, ver, 
                EJS_LOADER_STRICT /* UNUSED | EJS_LOADER_BUILTIN */);
    }
    return rc;
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
#ifdef BLD_MOD_NAME
    relModDir = mprAsprintf("%s/../%s", mprGetAppDir(ejs), BLD_MOD_NAME);
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, mprGetAbsPath(relModDir)));
#endif
#ifdef BLD_MOD_PREFIX
    ejsSetProperty(ejs, ap, -1, ejsCreatePathFromAsc(ejs, BLD_MOD_PREFIX));
#endif
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
    EjsService      *service;   
    Ejs             *ejs;
    Mpr             *mpr;
    int             status;

    status = 0;

    if ((mpr = mprCreate(0, NULL, 0)) != 0) {
        status = MPR_ERR_MEMORY;

    } else if ((service = ejsCreateService()) == 0) {
        status = MPR_ERR_MEMORY;

    } else if ((ejs = ejsCreateVm(NULL, NULL, 0, NULL, 0)) == 0) {
        status = MPR_ERR_MEMORY;

    } else if (ejsLoadModule(ejs, ejsCreateStringFromAsc(ejs, path), -1, -1, 0) < 0) {
        status = MPR_ERR_CANT_READ;

    } else if (ejsRun(ejs) < 0) {
        status = EJS_ERR;
    }
    mprDestroy(mpr);
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
    if (ejsRunFunction(ejs, fun, 0, 1, &args) == 0) {
        return EJS_ERR;
    }
    return 0;
}


int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, EjsObj *listener)
{
    EjsObj      *emitter, *argv[2];
    EjsArray    *list;
    int         i;

    if (*emitterPtr == 0) {
        *emitterPtr = ejsCreateInstance(ejs, ejsGetTypeByName(ejs, N("ejs", "Emitter")), 0, NULL);
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
    EjsObj      *argv[2];
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


int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *args)
{
    EjsObj  **av, **argv;
    int     i;

    if (emitter) {
        argv = args;
        av = mprAlloc((argc + 2) * sizeof(EjsObj*));
        av[0] = (EjsObj*) ejsCreateStringFromAsc(ejs, name);
        av[1] = thisObj ? thisObj : ejs->nullValue;
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


#if UNUSED && KEEP
typedef struct EjsLogData {
    Ejs         *ejs;
    EjsObj      *log;
    EjsFunction *loggerWrite;
    int         writeSlot;
} EjsLogData;


/*
    Set Mpr log data to go via ejs
 */
static int startLogging(Ejs *ejs)
{
    EjsLogData  *ld;
    EjsObj      *app;

    if ((ld = mprAllocObj(ejs, EjsLogData, NULL))  == 0) {
        return MPR_ERR_MEMORY;
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
#endif


static void logHandler(int flags, int level, cchar *msg)
{
    Mpr         *mpr;
    MprFile     *file;
    static int  solo = 0;
    char        *prefix, *tag, *amsg, lbuf[16], buf[MPR_MAX_STRING];

    //  MOB - not thread safe
    if (solo > 0) {
        return;
    }
    solo++;
    mpr = mprGetMpr();
    prefix = mpr->name;
    amsg = NULL;

    if (flags & MPR_ERROR_SRC) {
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
#if UNUSED && KEEP
    /*
        Direct Mpr log data via App.log
     */
    EjsLogData  *ld;
    EjsObj      *str, *saveException;
    ld = (EjsLogData*) mpr->altLogData;
    if (ld && ld->loggerWrite) {
        Ejs  *ejs;
        ejs = ld->ejs;
        str = (EjsObj*) ejsCreateStringFromAsc(ejs, msg);
        saveException = ejs->exception;
        ejsClearException(ejs);
        ejsRunFunction(ejs, ld->loggerWrite, ld->log, 1, &str);
        ejs->exception = saveException;

    } else 
#endif
    if (mpr->logData) {
        file = (MprFile*) mpr->logData;
        mprFprintf(file, "%s", msg);
    } else {
        file = (MprFile*) mpr->logData;
        mprPrintfError("%s", msg);
    }
    solo--;
}


int ejsStartMprLogging(char *logSpec)
{
    Mpr         *mpr;
    MprFile     *file;
    char        *levelSpec;
    int         level;

    level = 0;
    mpr = mprGetMpr();
    logSpec = sclone(logSpec);

    if ((levelSpec = strchr(logSpec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }
    if (strcmp(logSpec, "stdout") == 0) {
        file = mpr->fileSystem->stdOutput;

    } else if (strcmp(logSpec, "stderr") == 0) {
        file = mpr->fileSystem->stdError;

    } else {
        if ((file = mprOpenFile(logSpec, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
            mprPrintfError("Can't open log file %s\n", logSpec);
            return EJS_ERR;
        }
    }
    mprSetLogFd(mprGetFileFd(file));
    mprSetLogLevel(level);
    mprSetLogHandler(logHandler, (void*) file);
    return 0;
}


int ejsFreeze(Ejs *ejs, int freeze)
{
    int     old;

    if (ejs->state->fp) {
        old = ejs->state->fp->freeze;
    } else {
        old = ejs->freeze;
    }
    if (freeze != -1) {
        if (ejs->state->fp) {
            ejs->state->fp->freeze = freeze;
        } else {
            ejs->freeze = freeze;
        }
    }
    return old;
}


/*  
    Global memory allocation handler. This is invoked when there is no notifier to handle an allocation failure.
    The interpreter has an allocNotifier (see ejsService: allocNotifier) and it will handle allocation errors.
 */
static int allocNotifier(int flags, ssize size)
{
    EjsService  *sp;
    Ejs         *ejs;
    int         next;

    if (flags & MPR_MEM_DEPLETED) {
        mprPrintfError("Can't allocate memory block of size %d\n", size);
        mprPrintfError("Total memory used %d\n", (int) mprGetMem());
        exit(255);

    } else if (flags & MPR_MEM_LOW) {
        mprPrintfError("Memory request for %d bytes exceeds memory red-line\n", size);
        mprPrintfError("Total memory used %d\n", (int) mprGetMem());

    } else if (flags & MPR_MEM_YIELD) {
        sp = MPR->ejsService;
        //  MOB -- can this be a deadly embrace?
        mprLock(sp->mutex);
        for (next = 0; (ejs = mprGetNextItem(sp->vmlist, &next)) != 0; ) {
            ejs->yieldRequired = 1;
            ejsAttention(ejs);
        }
        mprUnlock(sp->mutex);
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
        char *name = mprGetMpr()->name;
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
    if (ejs->service->http == 0) {
        ejs->service->http = httpCreate(ejs->service);
    }
    ejs->http = ejs->service->http;
    ejsUnlockService(ejs);
}


void ejsSetSpecial(Ejs *ejs, int index, EjsAny *value)
{
    mprAssert(index < EJS_MAX_SPECIAL);
    ejs->values[index] = value;
}


EjsAny *ejsGetSpecial(Ejs *ejs, int index)
{
    mprAssert(index < EJS_MAX_SPECIAL);
    return ejs->values[index];
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
