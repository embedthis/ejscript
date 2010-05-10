/**
    ejsLoader.c - Ejscript module file file loader

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static int  addFixup(Ejs *ejs, int kind, EjsObj *target, int slotNum, EjsTypeFixup *fixup);
static int  alreadyLoaded(Ejs *ejs, cchar *name, int minVersion, int maxVersion);
static void createLoadState(Ejs *ejs, int flags);
static EjsTypeFixup *createFixup(Ejs *ejs, EjsName *qname, int slotNum);
static int  fixupTypes(Ejs *ejs, MprList *list);
static EjsObj *getCurrentBlock(Ejs *ejs, EjsModule *mp);
static int  getVersion(cchar *name);
static int  initializeModule(Ejs *ejs, EjsModule *mp, cchar *path);
static int  loadBlockSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadClassSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadDependencySection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadEndBlockSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadEndFunctionSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadEndClassSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadEndModuleSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadExceptionSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadFunctionSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static int  loadModFile(Ejs *ejs, cchar *filename, int minVersion, int maxVersion, int flags);
static EjsModule *loadModuleSection(Ejs *ejs, MprFile *file, EjsModuleHdr *hdr, int *created, int flags);
static int  loadSections(Ejs *ejs, MprFile *file, cchar *path, EjsModuleHdr *hdr, int flags);
static int  loadPropertySection(Ejs *ejs, MprFile *file, EjsModule *mp, int sectionType);
static int  loadScriptModule(Ejs *ejs, MprFile *file, cchar *path, int flags);
static char *makeModuleName(MprCtx ctx, cchar *name);
static int  readNumber(Ejs *ejs, MprFile *file, int *number);
static int  readWord(Ejs *ejs, MprFile *file, int *number);
static char *search(Ejs *ejs, cchar *filename, int minVersion, int maxVersion);
static int  swapWord(Ejs *ejs, int word);
static char *tokenToString(EjsModule *mp, int   token);
static int  trimModule(Ejs *ejs, char *name);

#if !BLD_FEATURE_STATIC
static int  loadNativeLibrary(Ejs *ejs, EjsModule *mp, cchar *path);
#endif

static int  loadDocSection(Ejs *ejs, MprFile *file, EjsModule *mp);
static void setDoc(Ejs *ejs, EjsModule *mp, EjsObj *block, int slotNum);

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
int ejsLoadModule(Ejs *ejs, cchar *path, int minVersion, int maxVersion, int flags)
{
    EjsModule       *mp;
    char            *trimmedPath, *name;
    int             nextModule, next, status, version;

    mprAssert(path && *path);

    trimmedPath = mprStrdup(ejs, path);
    if ((version = trimModule(ejs, trimmedPath)) != 0) {
        minVersion = maxVersion = version;
    }
    name = mprGetPathBase(ejs, trimmedPath);
    nextModule = mprGetListCount(ejs->modules);

    if ((status = alreadyLoaded(ejs, name, minVersion, maxVersion)) == 0) {
        createLoadState(ejs, flags);
        if (strcmp(name, "ejs") == 0) {
            flags |= EJS_LOADER_BUILTIN;
        }
        if ((status = loadModFile(ejs, trimmedPath, minVersion, maxVersion, flags)) == 0) {
            /*
                Do fixups and run initializers when all dependent modules are loaded. Solves forward ref problem.
             */
            if (fixupTypes(ejs, ejs->loadState->typeFixups) == 0) {
                //  MOB rationalize down to just ejs flag
                if (!ejs->empty && !(flags & EJS_LOADER_NO_INIT) && !(ejs->flags & EJS_FLAG_NO_INIT)) {
                    for (next = nextModule; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                        if ((status = initializeModule(ejs, mp, mp->path)) < 0) {
                            break;
                        }
                    }
                }
            }
        }
        mprFree(ejs->loadState);
        ejs->loadState = 0;
    }
    mprFree(trimmedPath);
    mprFree(name);
    return status;
}


static int loadModFile(Ejs *ejs, cchar *filename, int minVersion, int maxVersion, int flags)
{
    MprFile         *file;
    char            *path;
    int             status;

    mprAssert(filename && *filename);

    if ((path = search(ejs, filename, minVersion, maxVersion)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    if ((file = mprOpen(ejs, path, O_RDONLY | O_BINARY, 0666)) != NULL) {
        mprLog(ejs, 4, "Loading module %s", path);
        mprEnableFileBuffering(file, 0, 0);
        status = loadScriptModule(ejs, file, path, flags);
        mprFree(file);
    } else {
        ejsThrowIOError(ejs, "Can't open module file %s", path);
        status = MPR_ERR_CANT_OPEN;
    }
    mprFree(path);
    return status;
}


static int initializeModule(Ejs *ejs, EjsModule *mp, cchar *path)
{
    EjsNativeModule     *nativeModule;
    int                 priorGen;

    priorGen = 0;

    if (mp->hasNative && !mp->configured) {
        /*
            See if a native module initialization routine has been registered. If so, use that. Otherwise, look
            for a backing DSO.
         */
        if ((nativeModule = ejsLookupNativeModule(ejs, mp->name)) == 0) {
#if !BLD_FEATURE_STATIC
            if (loadNativeLibrary(ejs, mp, path) < 0) {
                if (ejs->exception == 0) {
                    ejsThrowIOError(ejs, "Can't load the native module file \"%s\"", path);
                }
                return MPR_ERR_CANT_INITIALIZE;
            }
            nativeModule = ejsLookupNativeModule(ejs, mp->name);
#endif
            if (!(ejs->flags & EJS_FLAG_NO_INIT)) {
                if (nativeModule->checksum != mp->checksum) {
                    ejsThrowIOError(ejs, "Module \"%s\" does not match native code (%d, %d)", path, nativeModule->checksum, 
                        mp->checksum);
                    return MPR_ERR_BAD_STATE;
                }
            }
        }
        if (nativeModule->flags & EJS_LOADER_ETERNAL) {
            priorGen = ejsSetGeneration(ejs, EJS_GEN_ETERNAL);
        }
        if (nativeModule && (nativeModule->callback)(ejs) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
        if (nativeModule->flags & EJS_LOADER_ETERNAL) {
            ejsSetGeneration(ejs, priorGen);
        }
        if (ejs->hasError || ejs->errorType == 0 || mprHasAllocError(ejs)) {
            if (!ejs->exception) {
                ejsThrowIOError(ejs, "Initialization error for %s (%d, %d)", path, ejs->hasError, mprHasAllocError(ejs));
            }
            return MPR_ERR;
        }
    }
    mp->configured = 1;
    if (ejsRunInitializer(ejs, mp) == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


static char *search(Ejs *ejs, cchar *filename, int minVersion, int maxVersion) 
{
    char        *path;

    mprAssert(filename && *filename);

    if ((path = ejsSearchForModule(ejs, filename, minVersion, maxVersion)) == 0) {
        mprLog(ejs, 2, "Can't find module file \"%s.mod\"", filename);
        if (minVersion < 0 || maxVersion < 0) {
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
    Load the sections: classes, properties and functions. Return the first module loaded in pup.
 */
static int loadSections(Ejs *ejs, MprFile *file, cchar *path, EjsModuleHdr *hdr, int flags)
{
    EjsModule   *mp;
    int         rc, sectionType, created;

    created = 0;
    mp = 0;

    while ((sectionType = mprGetc(file)) >= 0) {
        if (sectionType < 0 || sectionType >= EJS_SECT_MAX) {
            mprError(ejs, "Bad section type %d in %s", sectionType, mp->name);
            return EJS_ERR;
        }
        mprLog(ejs, 9, "Load section type %d", sectionType);

        rc = 0;
        switch (sectionType) {

        case EJS_SECT_BLOCK:
            rc = loadBlockSection(ejs, file, mp);
            break;

        case EJS_SECT_BLOCK_END:
            rc = loadEndBlockSection(ejs, file, mp);
            break;

        case EJS_SECT_CLASS:
            rc = loadClassSection(ejs, file, mp);
            break;

        case EJS_SECT_CLASS_END:
            rc = loadEndClassSection(ejs, file, mp);
            break;

        case EJS_SECT_DEPENDENCY:
            rc = loadDependencySection(ejs, file, mp);
            /*  Update the first free global (dependencies come before types, methods and properties of all kinds) */
            mp->firstGlobal = ejsGetPropertyCount(ejs, ejs->global);
            break;

        case EJS_SECT_EXCEPTION:
            rc = loadExceptionSection(ejs, file, mp);
            break;

        case EJS_SECT_FUNCTION:
            rc = loadFunctionSection(ejs, file, mp);
            break;

        case EJS_SECT_FUNCTION_END:
            rc = loadEndFunctionSection(ejs, file, mp);
            break;

        case EJS_SECT_MODULE:
            if ((mp = loadModuleSection(ejs, file, hdr, &created, flags)) != 0) {
                ejsAddModule(ejs, mp);
                mp->path = mprStrdup(mp, path);
            }
            mp->firstGlobal = (ejs->initialized) ? ejsGetPropertyCount(ejs, ejs->global) : 0;
            break;

        case EJS_SECT_MODULE_END:
            rc = loadEndModuleSection(ejs, file, mp);
            mp->lastGlobal = ejsGetPropertyCount(ejs, ejs->global);
            break;

        case EJS_SECT_PROPERTY:
            rc = loadPropertySection(ejs, file, mp, sectionType);
            break;

        case EJS_SECT_DOC:
            rc = loadDocSection(ejs, file, mp);
            break;

        default:
            mprAssert(0);
            return EJS_ERR;
        }
        if (rc < 0) {
            if (mp && mp->name && created) {
                ejsRemoveModule(ejs, mp);
                mprFree(mp);
            }
            return rc;
        }
    }
    return 0;
}


/*
    Load a module section and constant pool.
 */
static EjsModule *loadModuleSection(Ejs *ejs, MprFile *file, EjsModuleHdr *hdr, int *created, int flags)
{
    EjsModule   *mp;
    char        *pool, *name;
    int         rc, version, checksum, poolSize, nameToken;

    mprAssert(created);

    *created = 0;
    checksum = 0;

    /*
        We don't have the constant pool yet so we cant resolve the name yet.
     */
    rc = 0;
    rc += readNumber(ejs, file, &nameToken);
    rc += readNumber(ejs, file, &version);
    rc += readWord(ejs, file, &checksum);
    rc += readNumber(ejs, file, &poolSize);
    if (rc < 0 || poolSize <= 0 || poolSize > EJS_MAX_POOL) {
        return 0;
    }

    /*
        Read the string constant pool. The pool calls Steal when the module is created.
     */
    pool = (char*) mprAlloc(file, poolSize);
    if (pool == 0) {
        return 0;
    }
    if (mprRead(file, pool, poolSize) != poolSize) {
        mprFree(pool);
        return 0;
    }

    /*
        Convert module token into a name
     */
    if (nameToken < 0 || nameToken >= poolSize) {
        mprAssert(0);
        return 0;
    }
    name = &pool[nameToken];
    if (name == 0) {
        mprAssert(name);
        mprFree(pool);
        return 0;
    }

    mp = ejsCreateModule(ejs, name, version);
    if (mp == 0) {
        mprFree(pool);
        return 0;
    }
    ejsSetModuleConstants(ejs, mp, pool, poolSize);
    mp->scope = ejs->globalBlock;
    mp->checksum = checksum;
    *created = 1;

    if (strcmp(name, EJS_DEFAULT_MODULE) != 0) {
        /*
            Signify that loading the module has begun. We allow multiple loads into the default module.
         */
        mp->loaded = 1;
        mp->constants->locked = 1;
    }
    mp->file = file;
    mp->flags = flags;

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_MODULE, mp);
    }
    mprLog(ejs, 9, "Load module section %s", name);
    return mp;
}


static int loadEndModuleSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    mprLog(ejs, 9, "End module section %s", mp->name);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_MODULE_END, mp);
    }
    return 0;
}


static int loadDependencySection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsModule   *module;
    void        *saveCallback;
    char        *name;
    int         rc, next, minVersion, maxVersion, checksum, nextModule;

    mprAssert(ejs);
    mprAssert(file);
    mprAssert(mp);

    name = ejsModuleReadString(ejs, mp);
    ejsModuleReadNumber(ejs, mp, &checksum);
    ejsModuleReadNumber(ejs, mp, &minVersion);
    ejsModuleReadNumber(ejs, mp, &maxVersion);
    
    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    if (ejsLookupModule(ejs, name, minVersion, maxVersion) != 0) {
        return 0;
    }
    saveCallback = ejs->loaderCallback;
    nextModule = mprGetListCount(ejs->modules);
    ejs->loaderCallback = NULL;

    mprLog(ejs, 5, "    Load dependency section %s", name);
    rc = loadModFile(ejs, name, minVersion, maxVersion, mp->flags | EJS_LOADER_DEP);
    ejs->loaderCallback = saveCallback;
    if (rc < 0) {
        return rc;
    }
    if ((module = ejsLookupModule(ejs, name, minVersion, maxVersion)) != 0) {
        if (checksum != module->checksum) {
            ejsThrowIOError(ejs, "Can't load module %s.\n"
                "It was compiled using a different version of module %s.", 
                mp->name, name);
            return MPR_ERR_BAD_STATE;
        }
    }
    if (mp->dependencies == 0) {
        mp->dependencies = mprCreateList(mp);
    }
    for (next = nextModule; (module = mprGetNextItem(ejs->modules, &next)) != 0; ) {
        mprAddItem(mp->dependencies, module);
        if (ejs->loaderCallback) {
            (ejs->loaderCallback)(ejs, EJS_SECT_DEPENDENCY, mp, module);
        }
    }
    return 0;
}


static int loadBlockSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsBlock    *bp;
    EjsObj      *current;
    EjsName     qname;
    int         slotNum, numSlot;

    qname.space = EJS_BLOCK_NAMESPACE;
    qname.name = ejsModuleReadString(ejs, mp);
    ejsModuleReadNumber(ejs, mp, &slotNum);
    ejsModuleReadNumber(ejs, mp, &numSlot);

    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    bp = ejsCreateBlock(ejs, numSlot);
    ejsSetDebugName(bp, qname.name);
    current = getCurrentBlock(ejs, mp);

    /*
        TODO - replace this strict mode with dont-delete on a per property basis. Redefinition is then okay if the
        property to be replaced is !dont-delete
     */
    if (ejs->loadState->flags & EJS_LOADER_STRICT) {
        if (ejsLookupProperty(ejs, current, &qname) >= 0) {
            ejsThrowReferenceError(ejs, "Block \"%s\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    slotNum = ejsDefineProperty(ejs, current, slotNum, &qname, ejs->blockType, 0, (EjsObj*) bp);
    if (slotNum < 0) {
        return MPR_ERR_CANT_WRITE;
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_BLOCK, mp, current, slotNum, qname.name, numSlot, bp);
    }
    bp->scope = mp->scope;
    mp->scope = bp;
    return 0;
}


static int loadEndBlockSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    mprLog(ejs, 9, "    End block section %s", mp->name);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_BLOCK_END, mp);
    }
    mp->scope = mp->scope->scope;
    return 0;
}


static int loadClassSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsType         *type, *baseType, *iface, *nativeType;
    EjsTypeFixup    *fixup, *ifixup;
    EjsName         qname, baseClassName, ifaceClassName;
    EjsBlock        *block;
    int             attributes, numTypeProp, numInstanceProp, slotNum, numInterfaces, i;

    fixup = 0;
    ifixup = 0;
    
    qname.name = ejsModuleReadString(ejs, mp);
    qname.space = ejsModuleReadString(ejs, mp);

    ejsModuleReadNumber(ejs, mp, &attributes);
    ejsModuleReadNumber(ejs, mp, &slotNum);
    ejsModuleReadType(ejs, mp, &baseType, &fixup, &baseClassName, 0);
    ejsModuleReadNumber(ejs, mp, &numTypeProp);
    ejsModuleReadNumber(ejs, mp, &numInstanceProp);
    ejsModuleReadNumber(ejs, mp, &numInterfaces);

    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    if (ejs->loadState->flags & EJS_LOADER_STRICT) {
        if (ejsLookupProperty(ejs, ejs->global, &qname) >= 0) {
            ejsThrowReferenceError(ejs, "Class \"%s\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    if (fixup || (baseType && baseType->needFixup)) {
        attributes |= EJS_TYPE_FIXUP;
    }
    type = nativeType = 0;
    if (!ejs->empty) {
        if (attributes & EJS_PROP_NATIVE) {
            type = nativeType = (EjsType*) mprLookupHash(ejs->coreTypes, qname.name);
            if (type == 0) {
                mprLog(ejs, 1, "WARNING: can't find native type \"%s\"", qname.name);
            }
        } else {
#if BLD_DEBUG
            if (mprLookupHash(ejs->coreTypes, qname.name)) {
                mprError(ejs, "WARNING: type \"%s\" defined as a native type but not declared as native", qname.name);
            }
#endif
        }
    }
    if (attributes & EJS_TYPE_FIXUP) {
        baseType = 0;
        if (fixup == 0) {
            fixup = createFixup(ejs, (baseType) ? &baseType->qname : &ejs->objectType->qname, -1);
        }
    }
    mprLog(ejs, 9, "    Load %s class %s for module %s at slot %d", qname.space, qname.name, mp->name, slotNum);

    if (slotNum < 0) {
        slotNum = ejs->globalBlock->obj.numSlots;
    }
    if (type == 0) {
        type = ejsCreateType(ejs, &qname, mp, baseType, sizeof(EjsObj), slotNum, numTypeProp, numInstanceProp, attributes,0);
        if (type == 0) {
            ejsThrowInternalError(ejs, "Can't create class %s", qname.name);
            return MPR_ERR_BAD_STATE;
        }

    } else {
        if (ejsConfigureType(ejs, type, mp, baseType, numTypeProp, numInstanceProp, attributes) < 0) {
            ejsThrowInternalError(ejs, "Can't configure class %s", qname.name);
            return MPR_ERR_BAD_STATE;
        }
        mp->hasNative = 1;
#if FUTURE
        /*
            Currently errors on Namespace
         */
        if (attributes & EJS_TYPE_HAS_CONSTRUCTOR && !type->hasConstructor) {
            mprError(ejs, "WARNING: module indicates a constructor required but none exists for \"%s\"", type->qname.name);
        }
#endif
#if UNUSED && KEEP
        if (!type->native) {
            mprError(ejs, "WARNING: type not defined as native: \"%s\"", type->qname.name);
        }
#endif
    }
    
    /*
        Read implemented interfaces. Add to type->implements. Create fixup record if the interface type is not yet known.
     */
    if (numInterfaces > 0) {
        type->implements = mprCreateList(type);
        for (i = 0; i < numInterfaces; i++) {
            if (ejsModuleReadType(ejs, mp, &iface, &ifixup, &ifaceClassName, 0) < 0) {
                return MPR_ERR_CANT_READ;
            }
            if (iface) {
                mprAddItem(type->implements, iface);
            } else if (addFixup(ejs, EJS_FIXUP_INTERFACE_TYPE, (EjsObj*) type, -1, ifixup) < 0) {
                ejsThrowMemoryError(ejs);
                return MPR_ERR_NO_MEMORY;
            }
        }
    }
    if (mp->flags & EJS_LOADER_BUILTIN) {
        type->block.obj.builtin = 1;
    }
#if UNUSED && MOVED
    //  MOB -- push these into ejsConfigureType ejsCreateType
    if (attributes & EJS_TYPE_HAS_TYPE_INITIALIZER) {
        type->hasStaticInitializer = 1;
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
        type->dynamicInstance = 1;
    }
    if (attributes & EJS_TYPE_COPY_PROTOTYPE) {
        type->copyPrototype = 1;
    }
#endif
    slotNum = ejsDefineProperty(ejs, ejs->global, slotNum, &qname, ejs->typeType, attributes, (EjsObj*) type);
    if (slotNum < 0) {
        ejsThrowMemoryError(ejs);
        return MPR_ERR_NO_MEMORY;
    }
    type->module = mp;

    if (fixup) {
        if (addFixup(ejs, EJS_FIXUP_BASE_TYPE, (EjsObj*) type, -1, fixup) < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_NO_MEMORY;
        }
    }
    setDoc(ejs, mp, ejs->global, slotNum);

    block = (EjsBlock*) type;
    block->scope = mp->scope;
    mp->scope = block;

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_CLASS, mp, slotNum, qname, type, attributes);
    }
    return 0;
}


static int loadEndClassSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsType     *type;

    mprLog(ejs, 9, "    End class section");

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_CLASS_END, mp, mp->scope);
    }
    type = (EjsType*) mp->scope;
    if (type->prototype->hasScriptFunctions) {
        type->block.obj.hasScriptFunctions = 1;
    }
    if (type->block.obj.hasScriptFunctions && type->baseType) {
        ejsDefineTypeNamespaces(ejs, type);
    }
    mp->scope = mp->scope->scope;
    return 0;
}


static int loadFunctionSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsType         *returnType;
    EjsTypeFixup    *fixup;
    EjsFunction     *fun;
    EjsName         qname, returnTypeName;
    EjsObj          *current;
    uchar           *code;
    int             slotNum, numSlots, numArgs, numDefault, codeLen, numExceptions, attributes, strict, sn;

    strict = 0;
    qname.name = ejsModuleReadString(ejs, mp);
    qname.space = ejsModuleReadString(ejs, mp);

    ejsModuleReadNumber(ejs, mp, &attributes);
    ejsModuleReadByte(ejs, mp, &strict);
 
    ejsModuleReadType(ejs, mp, &returnType, &fixup, &returnTypeName, 0);
    ejsModuleReadNumber(ejs, mp, &slotNum);
    ejsModuleReadNumber(ejs, mp, &numSlots);
    ejsModuleReadNumber(ejs, mp, &numArgs);
    ejsModuleReadNumber(ejs, mp, &numDefault);
    ejsModuleReadNumber(ejs, mp, &numExceptions);
    ejsModuleReadNumber(ejs, mp, &codeLen);

    if (mp->hasError) {
        return MPR_ERR_CANT_READ;
    }
    current = getCurrentBlock(ejs, mp);
    if (ejsIsType(current) && !(attributes & EJS_PROP_STATIC)) {
        current = ((EjsType*) current)->prototype;
    }
    mprAssert(current);
    mprAssert(numArgs >= 0 && numArgs < EJS_MAX_ARGS);
    mprAssert(numExceptions >= 0 && numExceptions < EJS_MAX_EXCEPTIONS);

    mprLog(ejs, 9, "Loading function %s:%s at slot %d", qname.space, qname.name, slotNum);

    /*
        Read the code. We pass ownership of the code to createMethod i.e. don't free.
     */
    if (codeLen > 0) {
        code = (uchar*) mprAlloc(ejsGetAllocCtx(ejs), codeLen);
        if (code == 0) {
            return MPR_ERR_NO_MEMORY;
        }
        if (mprRead(file, code, codeLen) != codeLen) {
            mprFree(code);
            return MPR_ERR_CANT_READ;
        }
        current->hasScriptFunctions = 1;
    } else {
        code = 0;
    }
    if (attributes & EJS_PROP_NATIVE) {
        mp->hasNative = 1;
    }
    if (attributes & EJS_FUN_MODULE_INITIALIZER) {
        mp->hasInitializer = 1;
    }
    if (ejs->loadState->flags & EJS_LOADER_STRICT) {
        if ((sn = ejsLookupProperty(ejs, current, &qname)) >= 0 && !(attributes & EJS_FUN_OVERRIDE)) {
            if (!(attributes & EJS_TRAIT_SETTER && ejsHasTrait(current, sn, EJS_TRAIT_GETTER))) {
                if (ejsIsType(current)) {
                    ejsThrowReferenceError(ejs,
                        "function \"%s\" already defined in type \"%s\". Add \"override\" to the function declaration.", 
                        qname.name, ((EjsType*) current)->qname.name);
                } else {
                    ejsThrowReferenceError(ejs,
                        "function \"%s\" already defined. Try adding \"override\" to the function declaration.", qname.name);
                }
                return MPR_ERR_CANT_CREATE;
            }
        }
    }

#if UNUSED
    /*
        Create the function using the current scope chain
     */
    if (!ejsIsType(mp->scope) || !(attributes & EJS_PROP_STATIC)) {
        /* Instance method. Don't put type on the scope chain */
        scope = mp->scope->scope;
    } else {
        /* Type must be present on the scope chain */
        scope = mp->scope;
    }
#endif
    fun = ejsCreateFunction(ejs, qname.name, code, codeLen, numArgs, numDefault, numExceptions, returnType, attributes, 
        mp->constants, mp->scope, strict);
    if (fun == 0) {
        mprFree(code);
        return MPR_ERR_NO_MEMORY;
    }
    if (mp->flags & EJS_LOADER_BUILTIN) {
        fun->block.obj.builtin = 1;
    }
    if (code) {
        mprStealBlock(fun, code);
    }
    if (numSlots > 0) {
        fun->activation = ejsCreateActivation(ejs, fun, numSlots);
    }

    if (current == ejs->global && slotNum < 0) {
        //  MOB -- don't understand this. Why ejs->global and override?
        if (attributes & EJS_FUN_OVERRIDE) {
            slotNum = ejsLookupProperty(ejs, current, &qname);
            if (slotNum < 0) {
                mprError(ejs, "Can't find method \"%s\" to override", qname.name);
                return MPR_ERR_NO_MEMORY;
            }

        } else {
            slotNum = -1;
        }
    }
    if (attributes & EJS_FUN_MODULE_INITIALIZER && current == ejs->global) {
        mp->initializer = fun;
        slotNum = -1;
    } else {
        slotNum = ejsDefineProperty(ejs, current, slotNum, &qname, ejs->functionType, attributes, (EjsObj*) fun);
        if (slotNum < 0) {
            return MPR_ERR_NO_MEMORY;
        }
    }
    if (fixup) {
        mprAssert(returnType == 0);
        if (addFixup(ejs, EJS_FIXUP_RETURN_TYPE, (EjsObj*) fun, -1, fixup) < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_NO_MEMORY;
        }
    }
    setDoc(ejs, mp, current, slotNum);

    mp->currentMethod = fun;
    fun->block.scope = mp->scope;
    mp->scope = (EjsBlock*) fun;

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_FUNCTION, mp, current, slotNum, qname, fun, attributes);
    }
    return 0;
}


static int loadEndFunctionSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsFunction         *fun;

    mprLog(ejs, 9, "    End function section");

    fun = (EjsFunction*) mp->scope;
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_FUNCTION_END, mp, fun);
    }
    mp->scope = mp->scope->scope;
    return 0;
}


static int loadExceptionSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    EjsFunction         *fun;
    EjsType             *catchType;
    EjsTypeFixup        *fixup;
    EjsCode             *code;
    EjsEx               *ex;
    int                 tryStart, tryEnd, handlerStart, handlerEnd, numBlocks, numStack, flags, i;

    fun = mp->currentMethod;
    mprAssert(fun);

    flags = 0;
    code = &fun->body.code;

    for (i = 0; i < code->numHandlers; i++) {
        ejsModuleReadByte(ejs, mp, &flags);
        ejsModuleReadNumber(ejs, mp, &tryStart);
        ejsModuleReadNumber(ejs, mp, &tryEnd);
        ejsModuleReadNumber(ejs, mp, &handlerStart);
        ejsModuleReadNumber(ejs, mp, &handlerEnd);
        ejsModuleReadNumber(ejs, mp, &numBlocks);
        ejsModuleReadNumber(ejs, mp, &numStack);
        ejsModuleReadType(ejs, mp, &catchType, &fixup, 0, 0);
        if (mp->hasError) {
            return MPR_ERR_CANT_READ;
        }
        ex = ejsAddException(fun, tryStart, tryEnd, catchType, handlerStart, handlerEnd, numBlocks, numStack, flags, i);
        if (fixup) {
            mprAssert(catchType == 0);
            if (addFixup(ejs, EJS_FIXUP_EXCEPTION, (EjsObj*) ex, 0, fixup) < 0) {
                mprAssert(0);
                return MPR_ERR_NO_MEMORY;
            }
        }
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_EXCEPTION, mp, fun);
    }
    return 0;
}

/*
    Define a global, class or block property. Not used for function locals or args.
 */
static int loadPropertySection(Ejs *ejs, MprFile *file, EjsModule *mp, int sectionType)
{
    EjsType         *type;
    EjsTypeFixup    *fixup;
    EjsName         qname, propTypeName;
    EjsObj          *current, *value;
    cchar           *str;
    int             slotNum, attributes, fixupKind;

    value = 0;
    current = getCurrentBlock(ejs, mp);
    qname.name = ejsModuleReadString(ejs, mp);
    qname.space = ejsModuleReadString(ejs, mp);
    
    ejsModuleReadNumber(ejs, mp, &attributes);
    ejsModuleReadNumber(ejs, mp, &slotNum);
    ejsModuleReadType(ejs, mp, &type, &fixup, &propTypeName, 0);

    //  MOB -- remove the need for this flag

    if (attributes & EJS_PROP_HAS_VALUE) {
        if ((str = ejsModuleReadString(ejs, mp)) == 0) {
            return MPR_ERR_CANT_READ;
        }
        /*  Only doing for namespaces currently */
        value = (EjsObj*) ejsCreateNamespace(ejs, str, str);
    }

    mprLog(ejs, 9, "Loading property %s:%s at slot %d", qname.space, qname.name, slotNum);

    if (attributes & EJS_PROP_NATIVE) {
        mp->hasNative = 1;
    }
    if (ejs->loadState->flags & EJS_LOADER_STRICT) {
        if (ejsLookupProperty(ejs, current, &qname) >= 0) {
            ejsThrowReferenceError(ejs, "property \"%s\" already loaded", qname.name);
            return MPR_ERR_CANT_CREATE;
        }
    }
    if (ejsIsType(current) && !(attributes & EJS_PROP_STATIC) && current != ejs->global && ((EjsType*) current)->prototype){
        current = (EjsObj*) ((EjsType*) current)->prototype;
    }

    slotNum = ejsDefineProperty(ejs, current, slotNum, &qname, type, attributes, value);
    if (slotNum < 0) {
        return MPR_ERR_CANT_WRITE;
    }
    if (mp->flags & EJS_LOADER_BUILTIN) {
        value = ejsGetProperty(ejs, current, slotNum);
        value->builtin = 1;
    }

    if (fixup) {
        if (ejsIsFunction(current)) {
            fixupKind = EJS_FIXUP_LOCAL;
        } else if (ejsIsType(current) && !(attributes & EJS_PROP_STATIC) && current != ejs->global) {
            mprAssert(((EjsType*) current)->prototype);
            current = (EjsObj*) ((EjsType*) current)->prototype;
            fixupKind = EJS_FIXUP_INSTANCE_PROPERTY;
        } else {
            fixupKind = EJS_FIXUP_TYPE_PROPERTY;
        }
        mprAssert(type == 0);
        if (addFixup(ejs, fixupKind, current, slotNum, fixup) < 0) {
            ejsThrowMemoryError(ejs);
            return MPR_ERR_NO_MEMORY;
        }
    }
    setDoc(ejs, mp, current, slotNum);

    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_PROPERTY, mp, current, slotNum, qname, attributes, propTypeName);
    }
    return 0;
}


static int loadDocSection(Ejs *ejs, MprFile *file, EjsModule *mp)
{
    char        *doc;

    mprLog(ejs, 9, "    Documentation section");

    doc = ejsModuleReadString(ejs, mp);

    if (ejs->flags & EJS_FLAG_DOC) {
        mp->doc = doc;
        if (ejs->loaderCallback) {
            (ejs->loaderCallback)(ejs, EJS_SECT_DOC, doc);
        }
    }
    return 0;
}


#if !BLD_FEATURE_STATIC
/*
    Check if a native module exists at the given path. If so, load it. If the path is a scripted module
    but has a corresponding native module, then load that. Return 1 if loaded, -1 for errors, 0 if no
    native module found.
 */
static int loadNativeLibrary(Ejs *ejs, EjsModule *mp, cchar *modPath)
{
    MprModule   *mm;
    char        *bare, *path, initName[MPR_MAX_PATH], moduleName[MPR_MAX_PATH], *cp;

    /*
        Replace ".mod" with ".so", ".dll" or ".dylib"
     */
    bare = mprStrdup(ejs, modPath);
    if ((cp = strrchr(bare, '.')) != 0 && strcmp(cp, EJS_MODULE_EXT) == 0) {
        *cp = '\0';
    }
    path = mprStrcat(ejs, -1, bare, BLD_SHOBJ, NULL);
    mprFree(bare);

    if (! mprPathExists(ejs, path, R_OK)) {
        mprError(ejs, "Native module not found %s", path);
        mprFree(path);
        return MPR_ERR_CANT_ACCESS;
    }

    /*
        Build the DSO entry point name. Format is "Name_ModuleInit" where Name has "." converted to "_"
        Typical name: ejs_io_Init or com_acme_rockets_Init
     */
    mprStrcpy(moduleName, sizeof(moduleName), mp->name);
    moduleName[0] = tolower((int) moduleName[0]);
    mprSprintf(initName, sizeof(initName), "%s_Init", moduleName);
    for (cp = initName; *cp; cp++) {
        if (*cp == '.') {
            *cp = '_';
        }
    }
    mprLog(ejs, 4, "Loading native module %s", path);
    mm = mprLoadModule(ejs, path, initName, mp);
    mprFree(path);
    return (mm == 0) ? MPR_ERR_CANT_OPEN : 1;
}
#endif


/*
    Load a scripted module file
 */
static int loadScriptModule(Ejs *ejs, MprFile *file, cchar *path, int flags)
{
    EjsModuleHdr    hdr;
    int             status;

    mprAssert(path);

    /*
        Read module file header
     */
    if ((mprRead(file, &hdr, sizeof(hdr))) != sizeof(hdr)) {
        ejsThrowIOError(ejs, "Error reading module file %s, corrupt header", path);
        return EJS_ERR;
    }
    if ((int) swapWord(ejs, hdr.magic) != EJS_MODULE_MAGIC) {
        ejsThrowIOError(ejs, "Bad module file format in %s", path);
        return EJS_ERR;
    }
    if (swapWord(ejs, hdr.fileVersion) != EJS_MODULE_VERSION) {
        ejsThrowIOError(ejs, "Incompatible module file format in %s", path);
        return EJS_ERR;
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_START, path, &hdr);
    }

    /*
        Load the sections: classes, properties and functions. This may load multiple modules.
     */
    if ((status = loadSections(ejs, file, path, &hdr, flags)) < 0) {
        if (ejs->exception == 0) {
            ejsThrowReferenceError(ejs, "Can't load module file %s", path);
        }
        return status;
    }
    if (ejs->loaderCallback) {
        (ejs->loaderCallback)(ejs, EJS_SECT_END, ejs->modules, ejs->loadState->firstModule);
    }
    return 0;
}


static EjsObj *getCurrentBlock(Ejs *ejs, EjsModule *mp)
{
    EjsFunction     *fun;
    EjsObj          *block;
    
    block = (EjsObj*) mp->scope;
    mprAssert(block);

    if (ejsIsFunction(block)) {
        fun = (EjsFunction*) block;
        if (fun->activation) {
            return fun->activation;
        }
    }
    return block;
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
            type = (EjsType*) ejsGetProperty(ejs, ejs->global, fixup->typeSlotNum);

        } else if (fixup->typeName.name) {
            mprAssert(fixup->typeSlotNum < 0);
            type = (EjsType*) ejsGetPropertyByName(ejs, ejs->global, &fixup->typeName);
            
        } else {
            continue;
        }
        if (type == 0) {
            if (fixup->typeName.name) {
                ejsThrowReferenceError(ejs, "Can't fixup forward type reference for \"%s\". Fixup kind %d", 
                    fixup->typeName.name, fixup->kind);
            } else {
#if BLD_DEBUG
                ejsThrowReferenceError(ejs, "Can't fixup forward type reference for \"%s\". Fixup kind %d", 
                    fixup->target->name, fixup->kind);
#else
                ejsThrowReferenceError(ejs, "Can't fixup forward type reference for \"%s\". Fixup kind %d", 
                    fixup->typeName.name, fixup->kind);
#endif
            }
            return EJS_ERR;
        }

        switch (fixup->kind) {
        case EJS_FIXUP_BASE_TYPE:
            mprAssert(fixup->target);
            targetType = (EjsType*) fixup->target;
            targetType->needFixup = 1;
            ejsFixupType(ejs, targetType, type, 0);
            if (targetType->block.namespaces.length == 0 && type->block.obj.hasScriptFunctions) {
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
            trait = ejsGetPropertyTrait(ejs, fixup->target, fixup->slotNum);
            mprAssert(trait);
            if (trait) {
                trait->type = type;
            }
            break;

        case EJS_FIXUP_INSTANCE_PROPERTY:
            mprAssert(fixup->target);
            mprAssert(ejsIsBlock(fixup->target));
            mprAssert(fixup->target->isPrototype);
            trait = ejsGetPropertyTrait(ejs, fixup->target, fixup->slotNum);
            mprAssert(trait);
            if (trait) {
                trait->type = type;
            }
            break;

        case EJS_FIXUP_LOCAL:
            mprAssert(fixup->target);
            trait = ejsGetPropertyTrait(ejs, fixup->target, fixup->slotNum);
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
    major = (int) mprAtoi(name, 10);
    if ((tok = strchr(name, '.')) != 0) {
        minor = (int) mprAtoi(++tok, 10);
    }
    if ((tok = strchr(tok, '.')) != 0) {
        patch = (int) mprAtoi(++tok, 10);
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
    lastSlash = mprGetLastPathSeparator(ejs, name);
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
static char *probe(MprCtx ctx, cchar *path, int minVersion, int maxVersion)
{
    MprDirEntry     *dp, *best;
    MprList         *files;
    char            *dir, *base, *ext, *result;
    int             nameLen, version, next, bestVersion;

    mprAssert(ctx);
    mprAssert(path);

    mprLog(ctx, 7, "Probe for file %s", path);

    if (maxVersion == 0) {
        if (mprPathExists(ctx, path, R_OK)) {
            return mprStrdup(ctx, path);
        }
        return 0;
    }

    dir = mprGetPathDir(ctx, path);
    base = mprGetPathBase(ctx, path);
    if ((ext = strrchr(base, '.')) != 0) {
        *ext = '\0';
    }
    files = mprGetPathFiles(ctx, dir, 0);
    nameLen = strlen(base);
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
    if (best == 0) {
        result = 0;
    } else {
        result = mprJoinPath(ctx, dir, best->name);
    }
    mprFree(files);
    return result;
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
static char *searchForModule(Ejs *ejs, MprCtx ctx, cchar *moduleName, int minVersion, int maxVersion)
{
    MprCtx      *tx;
    EjsPath     *dir;
    char        *withDotMod, *path, *filename, *basename, *cp, *slash, *name, *bootSearch, *tok, *searchDir, *dp;
    int         i;

    mprAssert(moduleName && *moduleName);

    slash = 0;
    if (maxVersion <= 0) {
        maxVersion = MAXINT;
    }

    ctx = withDotMod = makeModuleName(ejs, moduleName);
    name = mprGetNormalizedPath(ctx, withDotMod);

    mprLog(ejs, 5, "Search for module \"%s\"", name);

    /*
        1. Search for path directly
     */
    if ((path = probe(ctx, name, minVersion, maxVersion)) != 0) {
        return path;
    }

    /*
        2. Search for "a/b/c"
     */
    slash = mprStrdup(ctx, name);
    for (cp = slash; *cp; cp++) {
        if (*cp == '.') {
            *cp = mprGetPathSeparator(ejs, name);
        }
    }
    if ((path = probe(ctx, slash, minVersion, maxVersion)) != 0) {
        return path;
    }

    if (ejs->search) {
        /*
            3. Search for "a.b.c" in EJSPATH
         */
        for (i = 0; i < ejs->search->length; i++) {
            dir = (EjsPath*) ejsGetProperty(ejs, (EjsVar*) ejs->search, i);
            if (!ejsIsPath(ejs, dir)) {
                continue;
            }
            filename = mprJoinPath(ctx, dir->path, name);
            if ((path = probe(ctx, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

        /*
            4. Search for "a/b/c" in EJSPATH
         */
        for (i = 0; i < ejs->search->length; i++) {
            dir = (EjsPath*) ejsGetProperty(ejs, (EjsVar*) ejs->search, i);
            if (!ejsIsPath(ejs, dir)) {
                continue;
            }
            filename = mprJoinPath(ctx, dir->path, slash);
            if ((path = probe(ctx, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

        /*
            5. Search for "c" in EJSPATH
         */
        basename = mprGetPathBase(ctx, slash);
        for (i = 0; i < ejs->search->length; i++) {
            dir = (EjsPath*) ejsGetProperty(ejs, (EjsVar*) ejs->search, i);
            if (!ejsIsPath(ejs, dir)) {
                continue;
            }
            filename = mprJoinPath(ctx, dir->path, basename);
            if ((path = probe(ctx, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }

    } else {
        /*
            Used when bootstrapping the VM
         */
        basename = mprGetPathBase(ctx, name);
        if (ejs->bootSearch) {
            bootSearch = mprStrdup(ejs, ejs->bootSearch);
            searchDir = mprStrTok(bootSearch, MPR_SEARCH_SEP, &tok);
            while (searchDir && *searchDir) {
                filename = mprJoinPath(ctx, searchDir, basename);
                if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                    mprFree(bootSearch);
                    return path;
                }
                searchDir = mprStrTok(NULL, MPR_SEARCH_SEP, &tok);
            }
            mprFree(bootSearch);

        } else {

            /* Search bin/../modules */
            dp = mprGetAppDir(ctx);
            tx = (MprCtx*) dp;
            dp = mprGetPathParent(tx, dp);
            dp = mprJoinPath(tx, dp, BLD_MOD_NAME);
            filename = mprJoinPath(ctx, dp, basename);
            mprFree(tx);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }

            /* Search bin */
            filename = mprJoinPath(ctx, mprGetAppDir(ctx), basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }

            /* Search "." */
            path = mprGetCurrentPath(ctx);
            filename = mprJoinPath(ctx, path, basename);
            if ((path = probe(ejs, filename, minVersion, maxVersion)) != 0) {
                return path;
            }
        }
    }
    return 0;
}


char *ejsSearchForModule(Ejs *ejs, cchar *moduleName, int minVersion, int maxVersion)
{
    MprCtx      ctx;
    char        *path, *withDotMod, *name;

    mprAssert(moduleName && *moduleName);

    if (maxVersion <= 0) {
        maxVersion = MAXINT;
    }
    ctx = withDotMod = makeModuleName(ejs, moduleName);
    name = mprGetNormalizedPath(ctx, withDotMod);

    mprLog(ejs, 5, "Search for module \"%s\"", name);
    path = searchForModule(ejs, ctx, name, minVersion, maxVersion);
    if (path) {
        mprLog(ctx, 5, "Found %s at %s", name, path);
    }
    mprFree(ctx);
    return path;
}


/*
    Ensure name has a ".mod" extension
 */
static char *makeModuleName(MprCtx ctx, cchar *name)
{
    char    *cp;

    if ((cp = strrchr(name, '.')) != NULL && strcmp(cp, EJS_MODULE_EXT) == 0) {
        return mprStrdup(ctx, name);
    }
    return mprStrcat(ctx, -1, name, EJS_MODULE_EXT, NULL);
}


static int alreadyLoaded(Ejs *ejs, cchar *name, int minVersion, int maxVersion)
{
    EjsModule   *mp;

    if ((mp = ejsLookupModule(ejs, name, minVersion, maxVersion)) == 0) {
        return 0;
    }
    if (mp->compiling && strcmp(name, EJS_DEFAULT_MODULE) != 0) {
        ejsThrowStateError(ejs, "Attempt to load module \"%s\" that is currently being compiled.", name);
        return MPR_ERR_ALREADY_EXISTS;
    }
    return 1;
}


static void createLoadState(Ejs *ejs, int flags)
{
    EjsLoadState    *ls;

    ls = ejs->loadState = mprAllocObjZeroed(ejs, EjsLoadState);
    ls->typeFixups = mprCreateList(ls);
    ls->firstModule = mprGetListCount(ejs->modules);
    ls->flags = flags;
}


/*
    Read a string constant. String constants are stored as token offsets into
    the constant pool. The pool contains null terminated UTF-8 strings.
 */
char *ejsModuleReadString(Ejs *ejs, EjsModule *mp)
{
    int     t;

    mprAssert(mp);

    if (ejsModuleReadNumber(ejs, mp, &t) < 0) {
        return 0;
    }
    return tokenToString(mp, t);
}


/*
    Read a type reference. Types are stored as either global property slot numbers or as strings (token offsets into the 
    constant pool). The lowest bit is set if the reference is a string. The type and name arguments are optional and may 
    be set to null. Return EJS_ERR for errors, otherwise 0. Return the 0 if successful, otherwise return EJS_ERR. If the 
    type could not be resolved, allocate a fixup record and return in *fixup. The caller should then call addFixup.
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
    if (ejsModuleReadNumber(ejs, mp, &t) < 0) {
        mprAssert(0);
        return EJS_ERR;
    }

    slot = -1;
    qname.name = 0;
    qname.space = 0;
    type = 0;

    switch (t & EJS_ENCODE_GLOBAL_MASK) {
    default:
        mp->hasError = 1;
        mprAssert(0);
        return EJS_ERR;

    case EJS_ENCODE_GLOBAL_NOREF:
        return 0;

    case EJS_ENCODE_GLOBAL_SLOT:
        /*
            Type is a builtin primitive type or we are binding globals.
         */
        slot = t >> 2;
        if (0 <= slot && slot < ejsGetPropertyCount(ejs, ejs->global)) {
            type = (EjsType*) ejsGetProperty(ejs, ejs->global, slot);
            if (type && (EjsObj*) type != ejs->nullValue) {
                qname = type->qname;
            }
        }
        break;

    case EJS_ENCODE_GLOBAL_NAME:
        /*
            Type was unbound at compile time
         */
        qname.name = tokenToString(mp, t >> 2);
        if (qname.name == 0) {
            mp->hasError = 1;
            mprAssert(0);
            return EJS_ERR;
        }
        if ((qname.space = ejsModuleReadString(ejs, mp)) == 0) {
            mp->hasError = 1;
            mprAssert(0);
            return EJS_ERR;
        }
        if (qname.name) {
            slot = ejsLookupProperty(ejs, ejs->global, &qname);
            if (slot >= 0) {
                type = (EjsType*) ejsGetProperty(ejs, ejs->global, slot);
            }
        }
        break;
    }
    if ((EjsObj*) type == ejs->nullValue) {
        type = 0;
    }
    if (type) {
        if (!ejsIsType(type)) {
            mp->hasError = 1;
            mprAssert(0);
            return EJS_ERR;
        }
        *typeRef = type;

    } else if (type == 0 && fixup) {
        *fixup = createFixup(ejs, &qname, slot);
    }
    if (typeName) {
        *typeName = qname;
    }
    if (slotNum) {
        *slotNum = slot;
    }
    return 0;
}


static EjsTypeFixup *createFixup(Ejs *ejs, EjsName *qname, int slotNum)
{
    EjsTypeFixup    *fixup;

    mprAssert(ejs->loadState->typeFixups);
    fixup = mprAllocZeroed(ejs->loadState->typeFixups, sizeof(EjsTypeFixup));
    if (fixup == 0) {
        return 0;
    }
    fixup->typeName = *qname;
    fixup->typeSlotNum = slotNum;
    return fixup;
}


static int addFixup(Ejs *ejs, int kind, EjsObj *target, int slotNum, EjsTypeFixup *fixup)
{
    int     index;

    mprAssert(ejs);
    mprAssert(fixup);
    mprAssert(ejs->loadState->typeFixups);

    fixup->kind = kind;
    fixup->target = target;
    fixup->slotNum = slotNum;

    index = mprAddItem(ejs->loadState->typeFixups, fixup);
    if (index < 0) {
        mprAssert(0);
        return EJS_ERR;
    }
    return 0;
}


/*
    Convert a token index into a string.
 */
static char *tokenToString(EjsModule *mp, int token)
{
    if (token < 0 || token >= mp->constants->len) {
        mprAssert(0);
        return 0;
    }
    mprAssert(mp->constants);
    if (mp->constants == 0) {
        mprAssert(0);
        return 0;
    }
    return &mp->constants->pool[token];
}


/*
    Decode an encoded 32-bit word
 */
int ejsDecodeWord(uchar **pp)
{
    uchar   *start;
    int     value;

    start = *pp;
    value = (int) ejsDecodeNum(pp);
    *pp = start + 4;
    return value;
}


/*
    Get an encoded 64 bit number. Variable number of bytes.
 */
int64 ejsDecodeNum(uchar **pp)
{
    uchar   *pos;
    uint64  t;
    uint    c;
    int     sign, shift;

    pos = *pp;
    c = (uint) *pos++;

    /*
        Map sign bit (0,1) to 1,-1
     */
    sign = 1 - ((c & 0x1) << 1);
    t = (c >> 1) & 0x3f;
    shift = 6;

    while (c & 0x80) {
        c = *pos++;
        t |= (c & 0x7f) << shift;
        shift += 7;
    }
    *pp = pos;
    return t * sign;
}


/*
    Decode a 4 byte number from a file
 */
static int readWord(Ejs *ejs, MprFile *file, int *number)
{
    uchar   buf[4], *pp;

    mprAssert(file);
    mprAssert(number);

    if (mprRead(file, buf, 4) != 4) {
        return MPR_ERR_CANT_READ;
    }
    pp = buf;
    *number = ejsDecodeWord(&pp);
    return 0;
}


/*
    Decode a number from a file. Same as ejsDecodeNum but reading from a file.
 */
static int readNumber(Ejs *ejs, MprFile *file, int *number)
{
    uint    t, c;
    int     sign, shift;

    mprAssert(file);
    mprAssert(number);

    if ((c = mprGetc(file)) < 0) {
        return MPR_ERR_CANT_READ;
    }

    /*
        Map sign bit (0,1) to 1,-1
     */
    sign = 1 - ((c & 0x1) << 1);
    t = (c >> 1) & 0x3f;
    shift = 6;
    
    while (c & 0x80) {
        if ((c = mprGetc(file)) < 0) {
            return MPR_ERR_CANT_READ;
        }
        t |= (c & 0x7f) << shift;
        shift += 7;
    }
    *number = (int) (t * sign);
    return 0;
}


double ejsDecodeDouble(uchar **pp)
{
    double   value;

    memcpy(&value, *pp, sizeof(double));
    *pp += sizeof(double);
    return value;
}


/*
    Encode a number in a RLL encoding. Encoding is:
        Bit     0:  Sign
        Bits  1-6:  Low 6 bits (0-64)
        Bit     7:  Extension bit
        Bits 8-15:  Next 7 bits
        Bits   16:  Extension bit
        ...
 */
int ejsEncodeNum(uchar *pos, int64 number)
{
    uchar       *start;
    uint        encoded;
    uint64      unumber;

    mprAssert(pos);

    start = pos;
    if (number < 0) {
        unumber = -number;
        encoded = (uint) (((unumber & 0x3F) << 1) | 1);
    } else {
        encoded = (uint) (((number & 0x3F) << 1));
        unumber = number;
    }
    unumber >>= 6;

    while (unumber) {
        *pos++ = encoded | 0x80;
        encoded = (int) (unumber & 0x7f);
        unumber >>= 7;
    }
    *pos++ = encoded;
    mprAssert((pos - start) < 11);
    return (int) (pos - start);
}


int ejsEncodeUint(uchar *pos, uint number)
{
    uchar       *start;
    uint        encoded;

    mprAssert(pos);

    start = pos;
    encoded = (uint) (((number & 0x3F) << 1));
    number >>= 6;

    while (number) {
        *pos++ = encoded | 0x80;
        encoded = (int) (number & 0x7f);
        number >>= 7;
    }
    *pos++ = encoded;
    mprAssert((pos - start) < 11);
    return (int) (pos - start);
}


/*
    Encode a 32-bit number. Always emit exactly 4 bytes.
 */
int ejsEncodeWord(uchar *pos, int number)
{
    int         len;

    mprAssert(pos);

    if (abs(number) > EJS_ENCODE_MAX_WORD) {
        mprAssert("Code generation error. Word exceeds maximum");
        return 0;
    }
    len = ejsEncodeNum(pos, (int64) number);
    mprAssert(len <= 4);
    return 4;
}


int ejsEncodeDouble(uchar *pos, double number)
{
#if UNUSED && OLD
    double   *ptr;
    ptr = (double*) pos;
    *ptr = number;
#else
    memcpy(pos, &number, sizeof(double));
#endif
    return sizeof(double);
}


int ejsEncodeByteAtPos(uchar *pos, int value)
{
    mprAssert(pos);

    *pos = value;
    return 0;
}


int ejsEncodeWordAtPos(uchar *pos, int value)
{
    mprAssert(pos);

    return ejsEncodeWord(pos, value);
}



//  TODO - refactor to return the number. Setting mp->hasError is sufficient
/*
    Read an encoded number. Numbers are little-endian encoded in 7 bits with
    the 0x80 bit of each byte being a continuation bit.
 */
int ejsModuleReadNumber(Ejs *ejs, EjsModule *mp, int *number)
{
    mprAssert(ejs);
    mprAssert(mp);
    mprAssert(number);

    if (readNumber(ejs, mp->file, number) < 0) {
        mp->hasError = 1;
        return -1;
    }
    return 0;
}


//  TODO - refactor to return the number. Setting mp->hasError is sufficient
int ejsModuleReadByte(Ejs *ejs, EjsModule *mp, int *number)
{
    int     c;

    mprAssert(mp);
    mprAssert(number);

    if ((c = mprGetc(mp->file)) < 0) {
        mp->hasError = 1;
        return MPR_ERR_CANT_READ;
    }
    *number = c;
    return 0;
}


static void setDoc(Ejs *ejs, EjsModule *mp, EjsObj *block, int slotNum)
{
    if (mp->doc && ejsIsBlock(block)) {
        ejsCreateDoc(ejs, (EjsBlock*) block, slotNum, mp->doc);
        mp->doc = 0;
    }
}


EjsDoc *ejsCreateDoc(Ejs *ejs, EjsBlock *block, int slotNum, cchar *docString)
{
    EjsDoc      *doc;
    char        key[32];

    doc = mprAllocZeroed(ejs, sizeof(EjsDoc));
    if (doc == 0) {
        return 0;
    }
    doc->docString = mprStrdup(doc, docString);
    if (ejs->doc == 0) {
        ejs->doc = mprCreateHash(ejs, EJS_DOC_HASH_SIZE);
    }

    /*
        This is slow, but not critical path
     */
    mprSprintf(key, sizeof(key), "%Lx %d", PTOL(block), slotNum);
    mprAddHash(ejs->doc, key, doc);
    return doc;
}


#if UNUSED
static int swapShort(Ejs *ejs, int word)
{
    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    word = ((word & 0xFFFF) << 16) | ((word & 0xFFFF0000) >> 16);
    return ((word & 0xFF) << 8) | ((word & 0xFF00) >> 8);
}
#endif


static int swapWord(Ejs *ejs, int word)
{
    if (mprGetEndian(ejs) == MPR_LITTLE_ENDIAN) {
        return word;
    }
    return ((word & 0xFF000000) >> 24) | ((word & 0xFF0000) >> 8) | ((word & 0xFF00) << 8) | ((word & 0xFF) << 24);
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
