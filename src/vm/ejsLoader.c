/**
    ejsLoader.c - Ejscript module file file loader

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static int  addFixup(Ejs *ejs, EjsModule *mp, int kind, EjsObj *target, int slotNum, EjsTypeFixup *fixup);
static int  alreadyLoaded(Ejs *ejs, EjsString *name, int minVersion, int maxVersion);
static EjsLoadState *createLoadState(Ejs *ejs, int flags);
static EjsTypeFixup *createFixup(Ejs *ejs, EjsModule *mp, EjsName qname, int slotNum);
static int  fixupTypes(Ejs *ejs, MprList *list);
static EjsObj *getCurrentBlock(EjsModule *mp);
static int  getVersion(cchar *name);
static int  initializeModule(Ejs *ejs, EjsModule *mp);
static int  loadBlockSection(Ejs *ejs, EjsModule *mp);
static int  loadClassSection(Ejs *ejs, EjsModule *mp);
static int  loadDependencySection(Ejs *ejs, EjsModule *mp);
static int  loadDocSection(Ejs *ejs, EjsModule *mp);
static int  loadEndBlockSection(Ejs *ejs, EjsModule *mp);
static int  loadEndFunctionSection(Ejs *ejs, EjsModule *mp);
static int  loadEndClassSection(Ejs *ejs, EjsModule *mp);
static int  loadEndModuleSection(Ejs *ejs, EjsModule *mp);
static int  loadDebugSection(Ejs *ejs, EjsModule *mp);
static int  loadExceptionSection(Ejs *ejs, EjsModule *mp);
static int  loadFunctionSection(Ejs *ejs, EjsModule *mp);
static EjsModule *loadModuleSection(Ejs *ejs, MprFile *file, EjsModuleHdr *hdr, int *created, int flags);
static int  loadNativeLibrary(Ejs *ejs, EjsModule *mp, cchar *path);
static int  loadSections(Ejs *ejs, MprFile *file, cchar *path, EjsModuleHdr *hdr, int flags);
static int  loadPropertySection(Ejs *ejs, EjsModule *mp, int sectionType);
static int  loadScriptModule(Ejs *ejs, cchar *filename, int minVersion, int maxVersion, int flags);
static char *makeModuleName(cchar *name);
static void popScope(EjsModule *mp, int keepScope);
static void pushScope(EjsModule *mp, EjsAny *block, EjsAny *obj);
static char *search(Ejs *ejs, cchar *filename, int minVersion, int maxVersion);
static int  trimModule(Ejs *ejs, char *name);
static void setDoc(Ejs *ejs, EjsModule *mp, cchar *tag, void *vp, int slotNum);

/******************************************************************************/
/**
    Load a module file and return a list of the loaded modules. This is used to load scripted module files with
    optional native (shared / DLL) implementations. If loading a scripted module that has native declarations, a
    search for the corresponding native DLL will be performed and both scripted and native module files will be loaded.
    NOTE: this may recursively call itself as it loads dependent modules.

    @param ejs Ejs handle
    @param path Module name or path to load. May be "." separated path. May include or omit the ".mod" extension.
    @param minVersion Minimum acceptable version (inclusive). Set to zero for unversioned.
    @param maxVersion Maximum acceptable version (inclusive). Set to -1 for all versions.
    @param flags Reserved. Must be set to zero.
    @param modulesArg List of modules loaded. Will only return a list if successful and doing a top level load. 
        When ejsLoadModule is called to load dependant modules, not list of modules will be returned.
        The final list of modules aggregates all modules loaded including those from dependant modules.
    @return Returns the last loaded module.
 */
int ejsLoadModule(Ejs *ejs, EjsString *path, int minVersion, int maxVersion, int flags)
{
    char    *trimmedPath, *name;
    int     status, version;

    mprAssert(path);

    /*
        Note the cannonical name for a module is the basename of the module without extension
     */
    trimmedPath = sclone(ejsToMulti(ejs, path));
    if ((version = trimModule(ejs, trimmedPath)) != 0) {
        minVersion = maxVersion = version;
    }
    name = mprGetPathBase(trimmedPath);

    if (flags & EJS_LOADER_RELOAD ||
            (status = alreadyLoaded(ejs, ejsCreateStringFromAsc(ejs, name), minVersion, maxVersion)) == 0) {
        status = loadScriptModule(ejs, trimmedPath, minVersion, maxVersion, flags);
    }
    return status;
}


static int initializeModule(Ejs *ejs, EjsModule *mp)
{
    EjsNativeModule     *nativeModule;
    int                 priorGen, old;

    priorGen = 0;

    if (mp->hasNative && !mp->configured) {
        /*
            See if a native module initialization routine has been registered. If so, use that. Otherwise, look
            for a backing shared library.
         */
        if ((nativeModule = ejsLookupNativeModule(ejs, ejsToMulti(ejs, mp->name))) == 0) {
            loadNativeLibrary(ejs, mp, mp->path);
            nativeModule = ejsLookupNativeModule(ejs, ejsToMulti(ejs, mp->name));
            if (nativeModule == NULL) {
                if (ejs->exception == 0) {
                    ejsThrowIOError(ejs, "Can't load or initialize the native module file \"%s\"", mp->path);
                }
                return MPR_ERR_CANT_INITIALIZE;
            }
            if (!(ejs->flags & EJS_FLAG_NO_INIT)) {
                if (nativeModule->checksum != mp->checksum) {
                    ejsThrowIOError(ejs, "Module \"%s\" does not match native code (%d, %d)", mp->path, 
                            nativeModule->checksum, mp->checksum);
                    return MPR_ERR_BAD_STATE;
                }
            }
        }
        if (nativeModule && (nativeModule->callback)(ejs) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
        if (ejs->hasError || ST(Error) == 0 || mprHasMemError(ejs)) {
            if (!ejs->exception) {
                ejsThrowIOError(ejs, "Initialization error for %s (%d, %d)", mp->path, ejs->hasError, mprHasMemError(ejs));
            }
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    mp->configured = 1;
    old = ejsFreeze(ejs, 1);
    if (ejsRunInitializer(ejs, mp) == 0) {
        ejsFreeze(ejs, old);
        return MPR_ERR_CANT_INITIALIZE;
    }
    ejsFreeze(ejs, old);
    return 0;
}


static char *search(Ejs *ejs, cchar *filename, int minVersion, int maxVersion) 
{
    char        *path;

    mprAssert(filename && *filename);

    if ((path = ejsSearchForModule(ejs, filename, minVersion, maxVersion)) == 0) {
        mprLog(2, "Can't find module file \"%s\"", filename);
        if (minVersion <= 0 && maxVersion <= 0) {
            ejsThrowReferenceError(ejs,  "Can't find module file \"%s\"", filename);
        } else if (minVersion == 0 && maxVersion == EJS_MAX_VERSION) {
            ejsThrowReferenceError(ejs,  "Can't find module file \"%s\"", filename);
        } else {
            ejsThrowReferenceError(ejs,  "Can't find module file \"%s\", min version %d.%d.%d, max version %d.%d.%d", 
                filename, 
                EJS_MAJOR(minVersion), EJS_MINOR(minVersion), EJS_PATCH(minVersion),
                EJS_MAJOR(maxVersion), EJS_MINOR(maxVersion), EJS_PATCH(maxVersion));
        }
        return 0;
    }
    return path;
}


/*
    Load the sections: modules, classes, properties and functions from a module file. May load muliple logical modules.
 */
static int loadSections(Ejs *ejs, MprFile *file, cchar *path, EjsModuleHdr *hdr, int flags)
{
    EjsModule   *mp;
    int         next, rc, sectionType, created, firstModule, status;

    created = 0;
    mp = 0;
    firstModule = mprGetListLength(ejs->modules);

    while ((sectionType = mprGetFileChar(file)) >= 0) {
        if (sectionType < 0 || sectionType >= EJS_SECT_MAX) {
            mprError("Bad section type %d in %@", sectionType, mp->name);
            return MPR_ERR_CANT_LOAD;
        }
        mprLog(9, "Load section type %d", sectionType);
        mprAssert(mp == NULL || mp->scope == NULL || mp->scope != mp->scope->scope);

        rc = 0;
        switch (sectionType) {

        case EJS_SECT_BLOCK:
            rc = loadBlockSection(ejs, mp);
            break;

        case EJS_SECT_BLOCK_END:
            rc = loadEndBlockSection(ejs, mp);
            break;

        case EJS_SECT_CLASS:
            rc = loadClassSection(ejs, mp);
            break;

        case EJS_SECT_CLASS_END:
            rc = loadEndClassSection(ejs, mp);
            break;

        case EJS_SECT_DEBUG:
            rc = loadDebugSection(ejs, mp);
            break;

        case EJS_SECT_DEPENDENCY:
            rc = loadDependencySection(ejs, mp);
            mp->firstGlobal = ejsGetPropertyCount(ejs, ejs->global);
            break;

        case EJS_SECT_EXCEPTION:
            rc = loadExceptionSection(ejs, mp);
            break;

        case EJS_SECT_FUNCTION:
            rc = loadFunctionSection(ejs, mp);
            break;

        case EJS_SECT_FUNCTION_END:
            rc = loadEndFunctionSection(ejs, mp);
            break;

        case EJS_SECT_MODULE:
            if ((mp = loadModuleSection(ejs, file, hdr, &created, flags)) == 0) {
                return MPR_ERR_CANT_LOAD;
            }
            ejsAddModule(ejs, mp);
            mp->path = sclone(path);
            mp->file = file;
            mp->firstGlobal = (ejs->initialized) ? ejsGetPropertyCount(ejs, ejs->global) : 0;
            break;

        case EJS_SECT_MODULE_END:
            rc = loadEndModuleSection(ejs, mp);
            mp->lastGlobal = ejsGetPropertyCount(ejs, ejs->global);
            break;

        case EJS_SECT_PROPERTY:
            rc = loadPropertySection(ejs, mp, sectionType);
            break;

        case EJS_SECT_DOC:
            rc = loadDocSection(ejs, mp);
            break;

        default:
            return MPR_ERR_CANT_LOAD;
        }
        if (rc < 0) {
            if (mp && mp->name && created) {
                ejsRemoveModule(ejs, mp);
            }
            return rc;
        }
    }
    status = 0;
    for (next = firstModule; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
        if (mp->loadState) {
            if (fixupTypes(ejs, mp->loadState->typeFixups) < 0) {
                return MPR_ERR_CANT_LOAD;
            }
            mp->loadState = 0;
        }
        //  TODO rationalize down to just ejs flag
        if (!ejs->empty && !(flags & EJS_LOADER_NO_INIT) && !(ejs->flags & EJS_FLAG_NO_INIT)) {
            if (!mp->initialized) {
                if ((status = initializeModule(ejs, mp)) < 0) {
                    break;
                }
            }
        }
    }
    if (ejs->loaderCallback && !ejs->exception) {
        (ejs->loaderCallback)(ejs, EJS_SECT_END, ejs->modules, firstModule);
    }
    return status;
}


/*
    Load a module section and constant pool.
    poolCount is the number of strings. poolSize is the size of the entire pool in bytes.
 */
static EjsConstants *loadConstants(Ejs *ejs, MprFile *file, int poolCount, int poolSize)
{
    EjsConstants    *constants;
    char            *pp;
    int             i;

    if ((constants = ejsCreateConstants(ejs, poolCount, poolSize)) == 0) {
        return 0;
    }
    if (mprReadFile(file, constants->pool, poolSize) != poolSize) {
        return 0;
    }
    constants->poolLength = poolSize;
    constants->indexCount = poolCount;
    for (pp = constants->pool, i = 1; pp < &constants->pool[constants->poolLength]; i++) {
        constants->index[i] = (void*) (((pp - constants->pool) << 1) | 0x1);
        pp += slen(pp) + 1;
    }
    return constants;
}


/*
    Load a module section and constant pool.
 */
static EjsModule *loadModuleSection(Ejs *ejs, MprFile *file, EjsModuleHdr *hdr, int *created, int flags)
{
    EjsModule       *mp, tmod;
    EjsConstants    *constants;
    EjsString       *name;
    int             version, checksum, poolSize, poolCount, nameToken;

    mprAssert(created);

    *created = 0;
    checksum = 0;

    /*
        We don't have the constant pool yet so we cant resolve the nameToken yet.
     */
    mp = &tmod;
    memset(&tmod, 0, sizeof(tmod));
    mp->file = file;
    nameToken = ejsModuleReadInt(ejs, mp);
    version   = ejsModuleReadInt(ejs, mp);
    checksum  = ejsModuleReadInt32(ejs, mp);
    poolSize  = ejsModuleReadInt(ejs, mp);
    poolCount = ejsModuleReadInt(ejs, mp);

    if (mp->hasError || poolSize <= 0 || poolSize > EJS_MAX_POOL) {
        return 0;
    }
    if (nameToken < 0 || nameToken >= poolSize) {
        mprAssert(0);
        return 0;
    }
    if ((constants = loadConstants(ejs, file, poolCount, poolSize)) == 0) {
        return 0;
    }
    mp->constants = constants;
    name = ejsCreateStringFromConst(ejs, mp, nameToken);
    if ((mp = ejsCreateModule(ejs, name, version, constants)) == NULL) {
        return 0;
    }
    mp->current = mprCreateList(-1, 0);
#if UNUSED
    pushScope(mp, ejs->global, ejs->global);
#else
    pushScope(mp, 0, ejs->global);
#endif
    mp->checksum = checksum;
    *created = 1;

    mp->file = file;
    mp->flags = flags;
    mp->loadState = createLoadState(ejs, flags);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_MODULE, mp);
    }
    mprLog(9, "Load module section %@", name);
    return mp;
}


static int loadEndModuleSection(Ejs *ejs, EjsModule *mp)
{
    mprLog(9, "End module section %@", mp->name);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_MODULE_END, mp);
    }
    mprAssert(mprGetListLength(mp->current) == 1);
    mp->current = 0;
    mp->file = 0;
    return 0;
}


static int loadDependencySection(Ejs *ejs, EjsModule *mp)
{
    EjsModule   *module;
    EjsString   *name;
    void        *saveCallback;
    int         next, rc, minVersion, maxVersion, checksum, nextModule;

    mprAssert(ejs);
    mprAssert(mp);

    name = ejsModuleReadConst(ejs, mp);
    checksum  = ejsModuleReadInt(ejs, mp);
    minVersion = ejsModuleReadInt(ejs, mp);
    maxVersion = ejsModuleReadInt(ejs, mp);
    
    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    if (ejsLookupModule(ejs, name, minVersion, maxVersion) == 0) {
        saveCallback = ejs->loaderCallback;
        nextModule = mprGetListLength(ejs->modules);
        ejs->loaderCallback = NULL;

        mprLog(6, "    Load dependency section %@", name);
        rc = loadScriptModule(ejs, ejsToMulti(ejs, name), minVersion, maxVersion, mp->flags | EJS_LOADER_DEP);
        ejs->loaderCallback = saveCallback;
        if (rc < 0) {
            return rc;
        }
        if (mp->dependencies == 0) {
            mp->dependencies = mprCreateList(-1, 0);
        }
        for (next = nextModule; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
            mprAddItem(mp->dependencies, module);
            if (ejs->loaderCallback) {
                (ejs->loaderCallback)(ejs, EJS_SECT_DEPENDENCY, mp, module);
            }
        }
    }
    if ((module = ejsLookupModule(ejs, name, minVersion, maxVersion)) != 0) {
        if (checksum != module->checksum) {
            ejsThrowIOError(ejs, "Can't load module \"%@\" due to checksum mismatch.\n"
                "The program was compiled depending on a different version of module \"%@\".", mp->name, name);
            return MPR_ERR_BAD_STATE;
        }
    }
    return 0;
}


static int loadBlockSection(Ejs *ejs, EjsModule *mp)
{
    EjsBlock    *bp;
    EjsObj      *current;
    EjsName     qname;
    int         slotNum, numSlot;

    qname.space = ejsCreateStringFromAsc(ejs, EJS_BLOCK_NAMESPACE);
    qname.name = ejsModuleReadConst(ejs, mp);
    slotNum = ejsModuleReadInt(ejs, mp);
    numSlot = ejsModuleReadInt(ejs, mp);

    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    bp = ejsCreateBlock(ejs, numSlot);
    mprSetName(bp, MPR_NAME("block"));
    current = getCurrentBlock(mp);

    /*
        TODO - replace this strict mode with dont-delete on a per property basis. Redefinition is then okay if the
        property to be replaced is !dont-delete
     */
    if (mp->loadState->flags & EJS_LOADER_STRICT) {
        if (ejsLookupProperty(ejs, current, qname) >= 0) {
            ejsThrowReferenceError(ejs, "Block \"%@\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    slotNum = ejsDefineProperty(ejs, current, slotNum, qname, ST(Block), 0, bp);
    if (slotNum < 0) {
        return MPR_ERR_CANT_WRITE;
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_BLOCK, mp, current, slotNum, qname.name, numSlot, bp);
    }
    pushScope(mp, bp, bp);
    return 0;
}


static int loadEndBlockSection(Ejs *ejs, EjsModule *mp)
{
    mprLog(9, "    End block section %@", mp->name);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_BLOCK_END, mp);
    }
    popScope(mp, 0);
    return 0;
}


static int loadClassSection(Ejs *ejs, EjsModule *mp)
{
    EjsType         *type, *baseType, *iface;
    EjsTypeFixup    *fixup, *ifixup;
    EjsName         qname, baseClassName, ifaceClassName;
    int             attributes, numTypeProp, numInstanceProp, numInterfaces, i, slotNum;

    fixup = 0;
    ifixup = 0;
    
    qname = ejsModuleReadName(ejs, mp);
    attributes = ejsModuleReadInt(ejs, mp);
    slotNum = ejsModuleReadInt(ejs, mp);
    ejsModuleReadType(ejs, mp, &baseType, &fixup, &baseClassName, 0);
    numTypeProp = ejsModuleReadInt(ejs, mp);
    numInstanceProp = ejsModuleReadInt(ejs, mp);
    numInterfaces = ejsModuleReadInt(ejs, mp);

    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    if (fixup || (baseType && baseType->needFixup)) {
        attributes |= EJS_TYPE_FIXUP;
    }

    /*
        Immutable types may be already loaded in global
     */
    type = ejsGetPropertyByName(ejs, ejs->global, qname);
#if 0
    if (type == 0) {
        type = ejsGetPropertyByName(ejs, ejs->immutable, qname);
    }
#endif

    if (type) {
        if (type->loaded && mp->loadState->flags & EJS_LOADER_STRICT) {
            ejsThrowReferenceError(ejs, "Class \"%@\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    if (type == 0 || !type->loaded) {
        if (attributes & EJS_TYPE_FIXUP) {
            baseType = 0;
            if (fixup == 0) {
                fixup = createFixup(ejs, mp, (baseType) ? baseType->qname : ST(Object)->qname, -1);
            }
        }
        mprLog(9, "    Load %@ class %@ for module %@ at slotNum %d", qname.space, qname.name, mp->name, slotNum);

        if (type == 0) {
            type = ejsCreateType(ejs, qname, mp, baseType, NULL, sizeof(EjsPot), -1, numTypeProp, numInstanceProp, 
                attributes);
            if (type == 0) {
                ejsThrowInternalError(ejs, "Can't create class %@", qname.name);
                return MPR_ERR_BAD_STATE;
            }
            ejsClonePotHelpers(ejs, type);

        } else {
            if (ejsConfigureType(ejs, type, mp, baseType, numTypeProp, numInstanceProp, attributes) < 0) {
                ejsThrowInternalError(ejs, "Can't configure class %@", qname.name);
                return MPR_ERR_BAD_STATE;
            }
            mp->hasNative = 1;
#if FUTURE
            /*
                Currently errors on Namespace
             */
            if (attributes & EJS_TYPE_HAS_CONSTRUCTOR && !type->hasConstructor) {
                mprError("WARNING: module indicates a constructor required but none exists for \"%@\"", type->qname.name);
            }
#endif
#if UNUSED && KEEP
            if (!type->native) {
                mprError("WARNING: type not defined as native: \"%@\"", type->qname.name);
            }
#endif
        }
    }
        
    /*
        Read implemented interfaces. Add to type->implements. Create fixup record if the interface type is not yet known.
     */
    if (numInterfaces > 0) {
        if (!type->implements) {
            type->implements = mprCreateList(numInterfaces, 0);
        }
        for (i = 0; i < numInterfaces; i++) {
            if (ejsModuleReadType(ejs, mp, &iface, &ifixup, &ifaceClassName, 0) < 0) {
                return MPR_ERR_CANT_READ;
            }
            if (!type->loaded) {
                if (iface) {
                    mprAddItem(type->implements, iface);
                } else if (addFixup(ejs, mp, EJS_FIXUP_INTERFACE_TYPE, (EjsObj*) type, -1, ifixup) < 0) {
                    ejsThrowMemoryError(ejs);
                    return MPR_ERR_MEMORY;
                }
            }
        }
    }
    if (!type->loaded) {
        slotNum = ejsDefineProperty(ejs, ejs->global, slotNum, qname, ST(Type), attributes, (EjsObj*) type);
        if (slotNum < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_MEMORY;
        }
        type->module = mp;
        if (fixup) {
            if (addFixup(ejs, mp, EJS_FIXUP_BASE_TYPE, (EjsObj*) type, -1, fixup) < 0) {
                ejsThrowMemoryError(ejs);
                return MPR_ERR_MEMORY;
            }
        }
        setDoc(ejs, mp, "class", ejs->global, slotNum);
    }
    pushScope(mp, type, type);
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_CLASS, mp, slotNum, qname, type, attributes);
    }
    if (type->loaded) {
        mprSeekFile(mp->file, SEEK_SET, type->endClass);
    }
    return 0;
}


static int loadEndClassSection(Ejs *ejs, EjsModule *mp)
{
    EjsType     *type;

    mprLog(9, "    End class section");

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_CLASS_END, mp, mp->scope);
    }
    type = (EjsType*) mp->scope;
    if (!type->loaded) {
        type->endClass = mprGetFilePosition(mp->file) - 1;
        if (type->hasScriptFunctions) {
            type->hasScriptFunctions = 1;
        }
        if (type->hasScriptFunctions && type->baseType) {
            ejsDefineTypeNamespaces(ejs, type);
        }
        popScope(mp, 0);
        if (type->dynamicInstances) {
            type->mutableInstances = 1;
        }
#if UNUSED 
        mprLog(0, "Type %N is %s and has %s instances", type->qname, (type->mutable) ? "mutable" : "immutable", type->mutableInstances ? "mutable" : "immutable");
#endif
    }
    type->loaded = 1;
    return 0;
}


static int loadFunctionSection(Ejs *ejs, EjsModule *mp)
{
    EjsType         *returnType, *currentType;
    EjsTypeFixup    *fixup;
    EjsFunction     *fun;
    EjsName         qname, returnTypeName;
    EjsPot          *block;
    uchar           *code;
    int             slotNum, numProp, numArgs, numDefault, codeLen, numExceptions, attributes, strict, sn;

    strict = 0;
    qname = ejsModuleReadName(ejs, mp);
    attributes = ejsModuleReadInt(ejs, mp);
    strict = ejsModuleReadByte(ejs, mp);
    ejsModuleReadType(ejs, mp, &returnType, &fixup, &returnTypeName, 0);
    slotNum = ejsModuleReadInt(ejs, mp);
    numProp = ejsModuleReadInt(ejs, mp);
    numArgs = ejsModuleReadInt(ejs, mp);
    numDefault = ejsModuleReadInt(ejs, mp);
    numExceptions = ejsModuleReadInt(ejs, mp);
    codeLen = ejsModuleReadInt(ejs, mp);
    
    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    block = (EjsPot*) getCurrentBlock(mp);
    currentType = 0;
    if (ejsIsType(ejs, block)) {
        currentType = (EjsType*) block;
        if (!(attributes & (EJS_FUN_CONSTRUCTOR | EJS_PROP_STATIC))) {
            block = ((EjsType*) currentType)->prototype;
        }
    }
    mprAssert(block);
    mprAssert(numArgs >= 0 && numArgs < EJS_MAX_ARGS);
    mprAssert(numExceptions >= 0 && numExceptions < EJS_MAX_EXCEPTIONS);

    mprLog(9, "Loading function %N at slot %d", qname, slotNum);

    /*
        Read the code
     */
    if (codeLen > 0) {
        if ((code = mprAlloc(codeLen)) == 0) {
            return MPR_ERR_MEMORY;
        }
        if (mprReadFile(mp->file, code, codeLen) != codeLen) {
            return MPR_ERR_CANT_READ;
        }
        if (currentType) {
            currentType->hasScriptFunctions = 1;
        }
    } else {
        code = 0;
    }
    if (attributes & EJS_PROP_NATIVE) {
        mp->hasNative = 1;
    }
    if (attributes & EJS_FUN_MODULE_INITIALIZER) {
        mp->hasInitializer = 1;
    }
    if (attributes & EJS_FUN_CONSTRUCTOR) {
        fun = (EjsFunction*) block;
        ejsInitFunction(ejs, fun, qname.name, code, codeLen, numArgs, numDefault, numExceptions, returnType, 
            attributes, mp, NULL, strict);
        mprAssert(fun->isConstructor);

    } else {
        if (mp->loadState->flags & EJS_LOADER_STRICT) {
            if ((sn = ejsLookupProperty(ejs, block, qname)) >= 0 && !(attributes & EJS_FUN_OVERRIDE)) {
                if (!(attributes & EJS_TRAIT_SETTER && ejsPropertyHasTrait(ejs, block, sn, EJS_TRAIT_GETTER))) {
                    if (ejsIsType(ejs, block)) {
                        ejsThrowReferenceError(ejs,
                            "function \"%@\" already defined in type \"%@\". Add \"override\" to the function declaration.", 
                            qname.name, ((EjsType*) block)->qname.name);
                    } else {
                        ejsThrowReferenceError(ejs,
                            "function \"%@\" already defined. Try adding \"override\" to the function declaration.", 
                            qname.name);
                    }
                    return MPR_ERR_CANT_CREATE;
                }
            }
        }
        fun = ejsCreateFunction(ejs, qname.name, code, codeLen, numArgs, numDefault, numExceptions, returnType, attributes, 
            mp, mp->scope, strict);
        if (fun == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    mprAssert(fun->block.pot.isBlock);
    mprAssert(fun->block.pot.isFunction);

    if (numProp > 0) {
        fun->activation = ejsCreateActivation(ejs, fun, numProp);
    }
    if (block == ejs->global && slotNum < 0) {
        if (attributes & EJS_FUN_OVERRIDE) {
            slotNum = ejsLookupProperty(ejs, block, qname);
            if (slotNum < 0) {
                mprError("Can't find method \"%@\" to override", qname.name);
                return MPR_ERR_MEMORY;
            }

        } else {
            slotNum = -1;
        }
    }
    if (!(attributes & EJS_FUN_CONSTRUCTOR)) {
        if (attributes & EJS_FUN_MODULE_INITIALIZER && block == ejs->global) {
            mp->initializer = fun;
            slotNum = -1;
        } else {
            slotNum = ejsDefineProperty(ejs, block, slotNum, qname, ST(Function), attributes, fun);
            if (slotNum < 0) {
                return MPR_ERR_MEMORY;
            }
        }
    }
    if (fixup) {
        mprAssert(returnType == 0);
        if (addFixup(ejs, mp, EJS_FIXUP_RETURN_TYPE, (EjsObj*) fun, -1, fixup) < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_MEMORY;
        }
    }
    if (currentType && attributes & EJS_FUN_CONSTRUCTOR) {
        setDoc(ejs, mp, "fun", ejs->global, ejsLookupProperty(ejs, ejs->global, currentType->qname));
    } else {
        setDoc(ejs, mp, "fun", block, slotNum);
    }

    mp->currentMethod = fun;
    pushScope(mp, ejsIsType(ejs, fun) ? NULL : fun, fun->activation);
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_FUNCTION, mp, block, slotNum, qname, fun, attributes);
    }
    return 0;
}


static int loadEndFunctionSection(Ejs *ejs, EjsModule *mp)
{
    EjsFunction     *fun;

    mprLog(9, "    End function section");

    fun = (EjsFunction*) mp->scope;
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_FUNCTION_END, mp, fun);
    }
    popScope(mp, ejsIsType(ejs, fun));
    return 0;
}


static int loadDebugSection(Ejs *ejs, EjsModule *mp)
{
    EjsFunction     *fun;
    int             size;

    fun = mp->currentMethod;
    mprAssert(fun);

    /* 
        Note the location in the file and skip over
     */
    mprAssert(!fun->isNativeProc);
    size = ejsModuleReadInt32(ejs, mp);
    fun->body.code->debugOffset = (int) mprGetFilePosition(mp->file);
    mprSeekFile(mp->file, SEEK_CUR, size);
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_DEBUG, mp, fun);
    }
    return 0;
}


static int loadExceptionSection(Ejs *ejs, EjsModule *mp)
{
    EjsFunction     *fun;
    EjsType         *catchType;
    EjsTypeFixup    *fixup;
    EjsCode         *code;
    EjsEx           *ex;
    int             tryStart, tryEnd, handlerStart, handlerEnd, numBlocks, numStack, flags, i;

    fun = mp->currentMethod;
    mprAssert(fun);

    flags = 0;
    code = fun->body.code;

    for (i = 0; i < code->numHandlers; i++) {
        flags        = ejsModuleReadByte(ejs, mp);
        tryStart     = ejsModuleReadInt(ejs, mp);
        tryEnd       = ejsModuleReadInt(ejs, mp);
        handlerStart = ejsModuleReadInt(ejs, mp);
        handlerEnd   = ejsModuleReadInt(ejs, mp);
        numBlocks    = ejsModuleReadInt(ejs, mp);
        numStack     = ejsModuleReadInt(ejs, mp);
        ejsModuleReadType(ejs, mp, &catchType, &fixup, 0, 0);
        if (mp->hasError) {
            return MPR_ERR_CANT_READ;
        }
        ex = ejsAddException(ejs, fun, tryStart, tryEnd, catchType, handlerStart, handlerEnd, numBlocks, numStack, flags, i);
        if (fixup) {
            mprAssert(catchType == 0);
            if (addFixup(ejs, mp, EJS_FIXUP_EXCEPTION, (EjsObj*) ex, 0, fixup) < 0) {
                mprAssert(0);
                return MPR_ERR_MEMORY;
            }
        }
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_EXCEPTION, mp, fun);
    }
    return 0;
}


static int loadPropertySection(Ejs *ejs, EjsModule *mp, int sectionType)
{
    EjsType         *type, *ctype;
    EjsTypeFixup    *fixup;
    EjsName         qname, propTypeName;
    EjsObj          *current, *value;
    int             slotNum, attributes, fixupKind;

    value = 0;
    current = getCurrentBlock(mp);
    qname = ejsModuleReadName(ejs, mp);
    
    attributes = ejsModuleReadInt(ejs, mp);
    slotNum = ejsModuleReadInt(ejs, mp);
    ejsModuleReadType(ejs, mp, &type, &fixup, &propTypeName, 0);

    /*
        This is used for namespace values. It is required when compiling (only) and thus module init code is not 
        being run -- but we still need the value of the namespace if a script wants to declare a variable qualified
        by the namespace that is defined in the module.
     */
    if (attributes & EJS_PROP_HAS_VALUE) {
        EjsString  *str;
        if ((str = ejsModuleReadConst(ejs, mp)) == 0) {
            return MPR_ERR_CANT_READ;
        }
        /*  Only doing for namespaces currently */
        value = (EjsObj*) ejsCreateNamespace(ejs, str);
    }
    mprLog(9, "Loading property %N at slot %d", qname, slotNum);

    if (attributes & EJS_PROP_NATIVE) {
        mp->hasNative = 1;
    }
    if (mp->loadState->flags & EJS_LOADER_STRICT) {
        if (ejsLookupProperty(ejs, current, qname) >= 0) {
            ejsThrowReferenceError(ejs, "property \"%@\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    if (ejsIsType(ejs, current)) {
        ctype = (EjsType*) current;
        if (!(attributes & EJS_TRAIT_READONLY)) {
            if (attributes & EJS_PROP_STATIC) {
                ctype->mutable = 1;
            } else {
                ctype->mutableInstances = 1;
            }
        }
        if (!(attributes & EJS_PROP_STATIC) && current != ejs->global && ctype->prototype) {
            current = (EjsObj*) ((EjsType*) current)->prototype;
        }
    }
    slotNum = ejsDefineProperty(ejs, current, slotNum, qname, type, attributes, value);
    if (slotNum < 0) {
        return MPR_ERR_CANT_WRITE;
    }
    if (fixup) {
        if (ejsIsFunction(ejs, current)) {
            fixupKind = EJS_FIXUP_LOCAL;
        } else if (ejsIsType(ejs, current) && !(attributes & EJS_PROP_STATIC) && current != ejs->global) {
            mprAssert(((EjsType*) current)->prototype);
            current = (EjsObj*) ((EjsType*) current)->prototype;
            fixupKind = EJS_FIXUP_INSTANCE_PROPERTY;
        } else {
            fixupKind = EJS_FIXUP_TYPE_PROPERTY;
        }
        mprAssert(type == 0);
        if (addFixup(ejs, mp, fixupKind, current, slotNum, fixup) < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_MEMORY;
        }
    }
    setDoc(ejs, mp, "var", current, slotNum);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_PROPERTY, mp, current, slotNum, qname, attributes, propTypeName);
    }
    return 0;
}


static int loadDocSection(Ejs *ejs, EjsModule *mp)
{
    EjsString   *doc;

    mprLog(9, "    Documentation section");

    doc = ejsModuleReadConst(ejs, mp);

    if (ejs->flags & EJS_FLAG_DOC) {
        mp->doc = doc;
        if (ejs->loaderCallback) {
            (ejs->loaderCallback)(ejs, EJS_SECT_DOC, doc);
        }
    }
    return 0;
}


/*
    Check if a native module exists at the given path. If so, load it. If the path is a scripted module
    but has a corresponding native module, then load that.
 */
static int loadNativeLibrary(Ejs *ejs, EjsModule *mp, cchar *modPath)
{
    MprModule   *native;
    char        *bare, *path, *moduleName, initName[MPR_MAX_PATH], *cp;

    /*
        Replace ".mod" with ".so", ".dll" or ".dylib"
     */
    bare = sclone(modPath);
    if ((cp = strrchr(bare, '.')) != 0 && strcmp(cp, EJS_MODULE_EXT) == 0) {
        *cp = '\0';
    }
    path = sjoin(bare, BLD_SHOBJ, NULL);

    if (! mprPathExists(path, R_OK)) {
        mprError("Native module not found %s", path);
        return MPR_ERR_CANT_ACCESS;
    }

    /*
        Build the DSO entry point name. Format is "Name_ModuleInit" where Name has "." converted to "_"
        Typical name: ejs_io_Init or com_acme_rockets_Init
     */
    moduleName = (char*) ejsToMulti(ejs, mp->name);
    moduleName[0] = tolower((int) moduleName[0]);
    mprSprintf(initName, sizeof(initName), "%s_Init", moduleName);
    for (cp = initName; *cp; cp++) {
        if (*cp == '.') {
            *cp = '_';
        }
    }
    mprLog(5, "Loading native module %s", path);
    native = mprCreateModule(mp->name->value, path, initName, ejs);
    if (mprLoadModule(native) < 0) {
        return MPR_ERR_CANT_READ;
    }
    return 0;
}


static int loadScriptModule(Ejs *ejs, cchar *filename, int minVersion, int maxVersion, int flags)
{
    EjsModuleHdr    hdr;
    EjsModule       *mp;
    MprFile         *file;
    char            *path;
    int             next, status, firstModule;

    mprAssert(filename && *filename);
    mprAssert(ejs->exception == 0);

    if ((path = search(ejs, filename, minVersion, maxVersion)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    if ((file = mprOpenFile(path, O_RDONLY | O_BINARY, 0666)) == NULL) {
        ejsThrowIOError(ejs, "Can't open module file %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprLog(5, "Loading module %s", path);
    mprEnableFileBuffering(file, 0, 0);
    firstModule = mprGetListLength(ejs->modules);

    /*
        Read module file header
     */
    status = 0;
    if ((mprReadFile(file, &hdr, sizeof(hdr))) != sizeof(hdr)) {
        ejsThrowIOError(ejs, "Can't read module file %s, corrupt header", path);
        status = MPR_ERR_CANT_LOAD;

    } else if ((int) ejsSwapInt32(ejs, hdr.magic) != EJS_MODULE_MAGIC) {
        ejsThrowIOError(ejs, "Bad module file format in %s", path);
        status = MPR_ERR_CANT_LOAD;

    } else if (ejsSwapInt32(ejs, hdr.fileVersion) != EJS_MODULE_VERSION) {
        ejsThrowIOError(ejs, "Incompatible module file format in %s", path);
        status = MPR_ERR_CANT_LOAD;

    } else {
        if (ejs->loaderCallback) {
            (ejs->loaderCallback)(ejs, EJS_SECT_START, path, &hdr);
        }
        if ((status = loadSections(ejs, file, path, &hdr, flags)) < 0) {
            if (ejs->exception == 0) {
                ejsThrowReferenceError(ejs, "Can't load module file %s", path);
                status = MPR_ERR_CANT_LOAD;
            }
        }
    }
    if (status) {
        for (next = firstModule; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
            ejsRemoveModule(ejs, mp);
        }
    }
    mprCloseFile(file);
    return status;
}


static int fixupTypes(Ejs *ejs, MprList *list)
{
    EjsTypeFixup    *fixup;
    EjsModule       *mp;
    EjsType         *type, *targetType;
    EjsTrait        *trait;
    EjsFunction     *targetFunction;
    EjsEx           *targetException;
    int             next;

    for (next = 0; (fixup = (EjsTypeFixup*) mprGetNextItem(list, &next)) != 0; ) {
        mp = 0;
        type = 0;
        if (fixup->typeSlotNum >= 0) {
            type = ejsGetProperty(ejs, ejs->global, fixup->typeSlotNum);
        } else if (fixup->typeName.name) {
            mprAssert(fixup->typeSlotNum < 0);
            type = ejsGetPropertyByName(ejs, ejs->global, fixup->typeName);
        } else {
            continue;
        }
        if (type == 0) {
            if (fixup->typeName.name) {
                ejsThrowReferenceError(ejs, "Can't fixup forward type reference for \"%@\". Fixup kind %d", 
                    fixup->typeName.name, fixup->kind);
            }
            return MPR_ERR_CANT_LOAD;
        }
        switch (fixup->kind) {
        case EJS_FIXUP_BASE_TYPE:
            mprAssert(fixup->target);
            targetType = (EjsType*) fixup->target;
            targetType->needFixup = 1;
            ejsFixupType(ejs, targetType, type, 0);
            if (targetType->constructor.block.namespaces.length == 0 && type->hasScriptFunctions) {
                ejsDefineTypeNamespaces(ejs, targetType);
            }
            break;

        case EJS_FIXUP_INTERFACE_TYPE:
            targetType = (EjsType*) fixup->target;
            mprAddItem(targetType->implements, type);
            break;

        case EJS_FIXUP_RETURN_TYPE:
            mprAssert(fixup->target);
            targetFunction = (EjsFunction*) fixup->target;
            targetFunction->resultType = type;
            break;

        case EJS_FIXUP_TYPE_PROPERTY:
            mprAssert(fixup->target);
            trait = ejsGetPropertyTraits(ejs, fixup->target, fixup->slotNum);
            mprAssert(trait);
            if (trait) {
                trait->type = type;
            }
            break;

        case EJS_FIXUP_INSTANCE_PROPERTY:
            mprAssert(fixup->target);
            mprAssert(ejsIsBlock(ejs, fixup->target));
            mprAssert(((EjsPot*) fixup->target)->isPrototype);
            trait = ejsGetPropertyTraits(ejs, fixup->target, fixup->slotNum);
            mprAssert(trait);
            if (trait) {
                trait->type = type;
            }
            break;

        case EJS_FIXUP_LOCAL:
            mprAssert(fixup->target);
            trait = ejsGetPropertyTraits(ejs, fixup->target, fixup->slotNum);
            mprAssert(trait);
            if (trait) {
                trait->type = type;
            }
            break;

        case EJS_FIXUP_EXCEPTION:
            mprAssert(fixup->target);
            targetException = (EjsEx*) fixup->target;
            targetException->catchType = type;
            break;

        default:
            mprAssert(0);
        }
    }
    return 0;
}


/*
    Parse a major.minor.version string
 */
int ejsParseModuleVersion(cchar *name)
{
    char    *tok;
    int     major, minor, patch;

    minor = patch = 0;
    major = (int) stoi(name, 10, NULL);
    if ((tok = strchr(name, '.')) != 0) {
        minor = (int) stoi(++tok, 10, NULL);
    }
    if ((tok = strchr(tok, '.')) != 0) {
        patch = (int) stoi(++tok, 10, NULL);
    }
    return EJS_MAKE_VERSION(major, minor, patch);
}


/*
    Remove "-version" and ".mod"
 */
static int trimModule(Ejs *ejs, char *name)
{
    cchar   *lastSlash;
    char    *cp, *vp;

    if ((cp = strrchr(name, '.')) != 0 && strcmp(cp, EJS_MODULE_EXT) == 0) {
        *cp = '\0';
    }
    if ((vp = strrchr(name, '-')) == 0) {
        return 0;
    }
    lastSlash = mprGetLastPathSeparator(name);
    if (lastSlash && lastSlash > vp) {
        /* There is a "-" but it is in the directory portion of the name and not in the module name */
        return 0;
    }
    *vp++ = '\0';
    return ejsParseModuleVersion(vp);
}


/*
    Extract the version from a module name
 */
static int getVersion(cchar *name)
{
    char    *vp;

    if ((vp = strrchr(name, '-')) == 0) {
        return 0;
    }
    return ejsParseModuleVersion(++vp);
}


/*
    Search for a file. If found, Return the path where the file was located. Otherwise return null.
 */
static char *probe(Ejs *ejs, cchar *path, int minVersion, int maxVersion)
{
    MprDirEntry     *dp, *best;
    MprList         *files;
    char            *dir, *base, *ext;
    int             next, nameLen, version, bestVersion;

    mprAssert(ejs);
    mprAssert(path);

    mprLog(7, "Probe for file %s", path);

    if (maxVersion == 0) {
        if (mprPathExists(path, R_OK)) {
            return sclone(path);
        }
        return 0;
    }
    dir = mprGetPathDir(path);
    base = mprGetPathBase(path);
    if ((ext = strrchr(base, '.')) != 0) {
        *ext++ = '\0';
    }
    files = mprGetPathFiles(dir, 0);
    nameLen = (int) strlen(base);
    bestVersion = -1;
    best = 0;

    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0; ) {
        if (strncmp(dp->name, base, nameLen) != 0) {
            continue;
        }
        if ((ext = strrchr(dp->name, '.')) == 0 || strcmp(ext, EJS_MODULE_EXT) != 0) {
            continue;
        }
        if (dp->name[nameLen] == '-') {
            version = getVersion(dp->name);
        } else if (&dp->name[nameLen] == ext) {
            version = 0;
        } else {
            continue;
        }
        if (version == 0 || (minVersion <= version && version <= maxVersion)) {
            if (best == 0 || bestVersion < version) {
                bestVersion = version;
                best = dp;
            }
        }
    }
    return (best == 0) ? 0 : mprJoinPath(dir, best->name);
}


/*
    Search for a module. moduleName is a filename or path name. It MUST have a ".mod" extension.

    The search strategy is: Given a name "a.b.c", scan for:

        1. File named a.b.c
        2. File named a/b/c
        3. File named a.b.c in EJSPATH
        4. File named a/b/c in EJSPATH
        5. File named c in EJSPATH
 */
static char *searchForModule(Ejs *ejs, cchar *moduleName, int minVersion, int maxVersion)
{
    EjsPath     *dir;
    char        *withDotMod, *path, *filename, *basename, *cp, *slash, *name, *bootSearch, *tok, *searchDir, *dp;
    int         i;

    mprAssert(moduleName && *moduleName);

    slash = 0;
    if (maxVersion <= 0) {
        maxVersion = MAXINT;
    }
    withDotMod = makeModuleName(moduleName);
    name = mprGetNormalizedPath(withDotMod);

    mprLog(6, "Search for module \"%s\"", name);

    /*
        1. Search for path directly
     */
    if ((path = probe(ejs, name, minVersion, maxVersion)) != 0) {
        return path;
    }

    /*
        2. Search for "a/b/c"
     */
    slash = sclone(name);
    for (cp = slash; *cp; cp++) {
        if (*cp == '.') {
            *cp = mprGetPathSeparators(name)[0];
        }
    }
    if ((path = probe(ejs, slash, minVersion, maxVersion)) != 0) {
        return path;
    }

    if (ejs->search) {
        /*
            3. Search for "a.b.c" in EJSPATH
         */
        for (i = 0; i < ejs->search->length; i++) {
            dir = ejsGetProperty(ejs, ejs->search, i);
            if (!ejsIs(ejs, dir, Path)) {
                continue;
            }
            filename = mprJoinPath(dir->value, name);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

        /*
            4. Search for "a/b/c" in EJSPATH
         */
        for (i = 0; i < ejs->search->length; i++) {
            dir = ejsGetProperty(ejs, ejs->search, i);
            if (!ejsIs(ejs, dir, Path)) {
                continue;
            }
            filename = mprJoinPath(dir->value, slash);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

        /*
            5. Search for "c" in EJSPATH
         */
        basename = mprGetPathBase(slash);
        for (i = 0; i < ejs->search->length; i++) {
            dir = ejsGetProperty(ejs, ejs->search, i);
            if (!ejsIs(ejs, dir, Path)) {
                continue;
            }
            filename = mprJoinPath(dir->value, basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

    } else {
        /*
            Used when bootstrapping the VM
         */
        basename = mprGetPathBase(name);
        if (ejs->bootSearch) {
            bootSearch = sclone(ejs->bootSearch);
            searchDir = stok(bootSearch, MPR_SEARCH_SEP, &tok);
            while (searchDir && *searchDir) {
                filename = mprJoinPath(searchDir, basename);
                if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                    return path;
                }
                searchDir = stok(NULL, MPR_SEARCH_SEP, &tok);
            }

        } else {
            /* Search bin/../modules */
            dp = mprGetAppDir();
            dp = mprGetPathParent(dp);
            dp = mprJoinPath(dp, BLD_MOD_NAME);
            filename = mprJoinPath(dp, basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
            /* Search bin */
            filename = mprJoinPath(mprGetAppDir(), basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
            /* Search "." */
            path = mprGetCurrentPath();
            filename = mprJoinPath(path, basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }
    }
    return 0;
}


char *ejsSearchForModule(Ejs *ejs, cchar *moduleName, int minVersion, int maxVersion)
{
    char        *path, *withDotMod, *name;

    mprAssert(moduleName && *moduleName);

    if (maxVersion <= 0) {
        maxVersion = MAXINT;
    }
    withDotMod = makeModuleName(moduleName);
    name = mprGetNormalizedPath(withDotMod);

    mprLog(6, "Search for module \"%s\"", name);
    path = searchForModule(ejs, name, minVersion, maxVersion);
    if (path) {
        mprLog(6, "Found %s at %s", name, path);
    }
    return path;
}


/*
    Ensure name has a ".mod" extension
 */
static char *makeModuleName(cchar *name)
{
    char    *cp;

    if ((cp = strrchr(name, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) == 0) {
        return sclone(name);
    }
    return sjoin(name, EJS_MODULE_EXT, NULL);
}


static int alreadyLoaded(Ejs *ejs, EjsString *name, int minVersion, int maxVersion)
{
    EjsModule   *mp;

    if ((mp = ejsLookupModule(ejs, name, minVersion, maxVersion)) == 0) {
        return 0;
    }
    if (mp->compiling && ejsCompareMulti(ejs, name, EJS_DEFAULT_MODULE) != 0) {
        ejsThrowStateError(ejs, "Attempt to load module \"%@\" that is currently being compiled.", name);
        return MPR_ERR_ALREADY_EXISTS;
    }
    return 1;
}


void manageLoadState(EjsLoadState *ls, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ls->typeFixups);
    }
}


static EjsLoadState *createLoadState(Ejs *ejs, int flags)
{
    EjsLoadState    *ls;

    ls = mprAllocObj(EjsLoadState, manageLoadState);
    ls->typeFixups = mprCreateList(-1, 0);
    ls->firstModule = mprGetListLength(ejs->modules);
    ls->flags = flags;
    return ls;
}


/*
    Read a type reference. Types are stored as either global property slot numbers or as strings (token offsets into the 
    constant pool). The lowest bit is set if the reference is a string. The type and name arguments are optional and may 
    be set to null. Return the 0 if successful, otherwise return < 0. If the type could not be resolved, allocate a 
    fixup record and return in *fixup. The caller should then call addFixup.
 */
int ejsModuleReadType(Ejs *ejs, EjsModule *mp, EjsType **typeRef, EjsTypeFixup **fixup, EjsName *typeName, int *slotNum)
{
    EjsType         *type;
    EjsName         qname;
    int             t, slot;

    mprAssert(mp);
    mprAssert(typeRef);
    mprAssert(fixup);

    *typeRef = 0;
    *fixup = 0;

    if (typeName) {
        typeName->name = 0;
        typeName->space = 0;
    }
    t = ejsModuleReadInt(ejs, mp);
    slot = -1;
    qname.name = 0;
    qname.space = 0;
    type = 0;

    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        mp->hasError = 1;
        return MPR_ERR_CANT_LOAD;

    case EJS_ENCODE_GLOBAL_NOREF:
        return 0;

    case EJS_ENCODE_GLOBAL_SLOT:
        /*
            Type is a builtin primitive type or we are binding globals.
         */
        slot = t >> 2;
        if (0 <= slot && slot < ejsGetPropertyCount(ejs, ejs->global)) {
            type = ejsGetProperty(ejs, ejs->global, slot);
            if (type && (EjsObj*) type != S(null)) {
                qname = type->qname;
            }
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        /*
            Type was unbound at compile time
         */
        qname.name = ejsCreateStringFromConst(ejs, mp, t >> 2);
        if (qname.name == 0) {
            mp->hasError = 1;
            return MPR_ERR_CANT_LOAD;
        }
        if ((qname.space = ejsModuleReadConst(ejs, mp)) == 0) {
            mp->hasError = 1;
            return MPR_ERR_CANT_LOAD;
        }
        if (qname.name) {
            slot = ejsLookupProperty(ejs, ejs->global, qname);
            if (slot >= 0) {
                type = ejsGetProperty(ejs, ejs->global, slot);
            }
        }
        break;
    }
    if (type == S(null)) {
        type = 0;
    }
    if (type) {
        if (!ejsIsType(ejs, type)) {
            mp->hasError = 1;
            return MPR_ERR_CANT_LOAD;
        }
        *typeRef = type;

    } else if (type == 0 && fixup) {
        *fixup = createFixup(ejs, mp, qname, slot);
    }
    if (typeName) {
        *typeName = qname;
    }
    if (slotNum) {
        *slotNum = slot;
    }
    return 0;
}


static EjsTypeFixup *createFixup(Ejs *ejs, EjsModule *mp, EjsName qname, int slotNum)
{
    EjsTypeFixup    *fixup;

    mprAssert(mp->loadState->typeFixups);

    /*
        Managed by manageLoadState
     */
    if ((fixup = mprAllocZeroed(sizeof(EjsTypeFixup))) == 0) {
        return 0;
    }
    fixup->typeName = qname;
    fixup->typeSlotNum = slotNum;
    return fixup;
}


static int addFixup(Ejs *ejs, EjsModule *mp, int kind, EjsObj *target, int slotNum, EjsTypeFixup *fixup)
{
    int     index;

    mprAssert(ejs);
    mprAssert(fixup);
    mprAssert(mp->loadState->typeFixups);

    fixup->kind = kind;
    fixup->target = target;
    fixup->slotNum = slotNum;

    index = mprAddItem(mp->loadState->typeFixups, fixup);
    if (index < 0) {
        return MPR_ERR_CANT_LOAD;
    }
    return 0;
}


static void setDoc(Ejs *ejs, EjsModule *mp, cchar *tag, void *vp, int slotNum)
{
    if (mp->doc) {
        ejsCreateDoc(ejs, tag, vp, slotNum, mp->doc);
        mp->doc = 0;
    }
}


static void manageDoc(EjsDoc *doc, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(doc->docString);
        mprMark(doc->brief);
        mprMark(doc->description);
        mprMark(doc->example);
        mprMark(doc->requires);
        mprMark(doc->returns);
        mprMark(doc->stability);
        mprMark(doc->spec);
        mprMark(doc->duplicate);
        mprMark(doc->defaults);
        mprMark(doc->params);
        mprMark(doc->options);
        mprMark(doc->events);
        mprMark(doc->see);
        mprMark(doc->throws);
    }
}


EjsDoc *ejsCreateDoc(Ejs *ejs, cchar *tag, void *vp, int slotNum, EjsString *docString)
{
    EjsDoc      *doc;
    char        key[32];

    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(EJS_DOC_HASH_SIZE, 0);
    }
    mprSprintf(key, sizeof(key), "%s %Lx %d", tag, PTOL(vp), slotNum);
    if ((doc = mprLookupKey(ejs->doc, key)) != 0) {
        return doc;
    }
    if ((doc = mprAllocObj(EjsDoc, manageDoc)) == 0) {
        return 0;
    }
    doc->docString = docString;
    mprAddKey(ejs->doc, key, doc);
    return doc;
}


static EjsObj *getCurrentBlock(EjsModule *mp)
{
    return mprGetLastItem(mp->current);
}


static void pushScope(EjsModule *mp, EjsAny *block, EjsAny *obj)
{
    if (block) {
        mprAssert(block != mp->scope);
        ((EjsBlock*) block)->scope = mp->scope;
        mp->scope = block;
        mprAssert(mp->scope != mp->scope->scope);
    }
    mprPushItem(mp->current, obj);
}


static void popScope(EjsModule *mp, int keepScope)
{
    mprPopItem(mp->current);
    if (!keepScope) {
        mprAssert(mp->scope != mp->scope->scope);
        mp->scope = mp->scope->scope;
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
