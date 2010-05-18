/**
    ejsType.c - Type class

    The type class is the base class for all types (classes) in the system.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static void fixInstanceSize(Ejs *ejs, EjsType *type);
static int fixupPrototypeProperties(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom);
static int inheritProperties(Ejs *ejs, EjsType *type, EjsObj *obj, int destOffset, EjsObj *baseBlock, int srcOffset, 
    int count, bool resetScope);
static void setAttributes(EjsType *type, int64 attributes);
static EjsObj *type_prototype(Ejs *ejs, EjsType *type, int argc, EjsObj **argv);

/******************************************************************************/
/*
    Copy a type. 

    function copy(type: Object): Object
 */
static EjsType *cloneTypeVar(Ejs *ejs, EjsType *src, bool deep)
{
    EjsType     *dest;

    if (! ejsIsType(src)) {
        ejsThrowTypeError(ejs, "Expecting a Type object");
        return 0;
    }
    dest = (EjsType*) (ejs->blockType->helpers.clone)(ejs, (EjsObj*) src, deep);
    if (dest == 0) {
        return dest;
    }
    //  TODO OPT
    dest->baseType = src->baseType;
    dest->callsSuper = src->callsSuper;
    dest->dontPool = src->dontPool;
    dest->final = src->final;
    dest->hasConstructor = src->hasConstructor;
    dest->hasBaseConstructors = src->hasBaseConstructors;
    dest->hasBaseInitializers = src->hasBaseInitializers;
    dest->hasMeta = src->hasMeta;
    dest->hasInitializer = src->hasInitializer;
    dest->helpers = src->helpers;
    dest->id = src->id;
    dest->immutable = src->immutable;
    dest->initialized = src->initialized;
    dest->instanceSize = src->instanceSize;
    dest->isInterface = src->isInterface;
    dest->module = src->module;
    dest->numericIndicies = src->numericIndicies;
    dest->numInherited = src->numInherited;
    dest->prototype = src->prototype;
    dest->qname = src->qname;
    dest->typeData = src->typeData;
    return dest;
}


/*
    Create a new Type object. numSlots is the number of property slots to pre-allocate.
    This is hand-crafted to create types as small as possible.
 */
static EjsType *createTypeVar(Ejs *ejs, EjsType *typeType, int numSlots)
{
    EjsType         *type;
    EjsObj          *obj;
    EjsObj          *vp;
    char            *start;
    int             typeSize, sizeHash, dynamic;

    mprAssert(ejs);

    /*
        If the compiler is building itself (empty mode), then the types themselves must be dynamic. Otherwise, the type
        is fixed and will contain the names hash and traits in one memory block. 
        NOTE: don't confuse this with dynamic objects.
     */
    sizeHash = 0;

    if (numSlots < 0 || ejs->empty || ejs->flags & EJS_FLAG_DYNAMIC) {
        dynamic = 1;
        typeSize = sizeof(EjsType);
        numSlots = 0;

    } else {
        dynamic = 0;
        typeSize = sizeof(EjsType);
        typeSize += (int) sizeof(EjsSlot) * numSlots;
        if (numSlots > EJS_HASH_MIN_PROP) {
            sizeHash = ejsGetHashSize(numSlots);
            typeSize += sizeof(EjsHash) + (sizeHash * (int) sizeof(EjsSlot*));
        }
    }
    if ((vp = (EjsObj*) mprAllocZeroed(ejsGetAllocCtx(ejs), typeSize)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    obj = (EjsObj*) vp;
    type = (EjsType*) vp;
    vp->type = type;
    vp->master = (ejs->master == 0);
    vp->type = typeType;
    vp->isType = 1;
    vp->dynamic = dynamic;
    ejsInitList(&type->constructor.block.namespaces);

    if (!dynamic) {
        /*
            This is for a fixed type. This is the normal case when not compiling. Layout is:

            Slots: sizeof(EjsSlot) * numSlots
            Hash:  ejsGetHashSize(numslots)
         */
        start = (char*) type + sizeof(EjsType);

        if (numSlots > 0) {
            obj->sizeSlots = numSlots;
            obj->slots = (EjsSlot*) start;
            ejsZeroSlots(ejs, obj->slots, numSlots);
            start += sizeof(EjsSlot) * numSlots;
        }
        if (sizeHash > 0) {
            obj->hash = (EjsHash*) start;
            obj->hash->buckets = (int*) (start + sizeof(EjsHash));
            obj->hash->size = sizeHash;
            memset(obj->hash->buckets, -1, sizeHash * sizeof(int));
            start += sizeof(EjsHash) + sizeof(int) * sizeHash;
        }
        mprAssert((start - (char*) type) <= typeSize);
    }
    ejsSetDebugName(vp, "type");
    ejsAddToGcStats(ejs, vp, ES_Type);
    return type;
}


/*
    Lookup a property with a namespace qualifier in an object and return the slot if found. Return EJS_ERR if not found.
 */
static int lookupTypeProperty(struct Ejs *ejs, EjsType *type, EjsName *qname)
{
    int         slotNum;

    slotNum = (ejs->objectType->helpers.lookupProperty)(ejs, (EjsObj*) type, qname);
#if UNUSED
    if (slotNum < 0 && strcmp(qname->name, "prototype") == 0 && (qname->space == NULL || qname->space[0] == '\0')) {
#if OLD
        /*
            On-demand creation of the Type.prototype. Replace Object.prototype getter for this type.
         */
        if (type->prototype == 0) {
            type->prototype = ejsCreatePrototype(ejs, type, 0);
        }
#endif
        ejsDefineProperty(ejs, (EjsObj*) type, ES_Object_prototype, ejsName(&name, type->qname.space, qname->name),
            ejs->objectType, 0, (EjsObj*) type->prototype);
    }
#endif
    return slotNum;
}


void markType(Ejs *ejs, EjsType *type)
{
    ejsMarkBlock(ejs, (EjsBlock*) type);

    if (type->prototype) {
        ejsMark(ejs, type->prototype);
    }
    if (type->baseType) {
        ejsMark(ejs, (EjsObj*) type->baseType);
    }
}


static int setTypeProperty(Ejs *ejs, EjsType *type, int slotNum, EjsObj *value)
{
    if (slotNum < 0 && !type->constructor.block.obj.dynamic) {
        ejsThrowTypeError(ejs, "Object is not dynamic");
        return EJS_ERR;
    }
    return (ejs->blockType->helpers.setProperty)(ejs, (EjsObj*) type, slotNum, value);
}


/******************************** Native Type API *****************************/
/*
    Create a core built-in type. This is used by core native type code to either create a type or to get a type
    that has been made by loading ejs.mod. Handles the EMPTY case when building the compiler itself.
 */
EjsType *ejsCreateCoreType(Ejs *ejs, EjsName *qname, EjsType *baseType, int instanceSize, int id, int numTypeProp,
    int numInstanceProp, int64 attributes)
{
    EjsType     *type;

    type = ejsCreateType(ejs, qname, NULL, baseType, NULL, instanceSize, id, numTypeProp, numInstanceProp, attributes, 0);
    if (type == 0) {
        ejs->hasError = 1;
        return 0;
    }
    /*
        The coreTypes hash allows the loader to match the essential core type objects to those being loaded from a mod file.
     */
    mprAddHash(ejs->coreTypes, qname->name, type);
    return type;
}


EjsType *ejsCreateNativeType(Ejs *ejs, cchar *space, cchar *name, int id, int instanceSize)
{
    EjsName     qname;

    return ejsCreateCoreType(ejs, ejsName(&qname, space, name), NULL, instanceSize, id, 0, 0, 0);
}


EjsType *ejsConfigureNativeType(Ejs *ejs, cchar *space, cchar *name, int instanceSize)
{
    EjsType     *type;

    if ((type = ejsGetTypeByName(ejs, space, name)) == 0) {
        mprError(ejs, "Can't find %s type", name);
        return 0;
    }
    type->instanceSize = instanceSize;
    return type;
}


EjsType *ejsCreateTypeFromFunction(Ejs *ejs, EjsFunction *fun, EjsObj *prototype)
{
    EjsName     qname;
    EjsType     *type;
    EjsTrait    *trait;
    EjsFunction *constructor;
    EjsCode     *code;
    int         slotNum;

    slotNum = ejsGetPropertyCount(ejs, ejs->global);

    type = ejsCreateType(ejs, ejsName(&qname, EJS_PROTOTYPE_NAMESPACE, fun->name), NULL, ejs->objectType, prototype,
        ejs->objectType->instanceSize, slotNum, 0, 0, EJS_TYPE_DYNAMIC_INSTANCE | EJS_TYPE_HAS_CONSTRUCTOR, NULL);
    if (type == 0) {
        return 0;
    }

    /*
        Type is installed, but not hashed in the global names
     */
    ejsSetProperty(ejs, ejs->global, slotNum, (EjsObj*) type);
    trait = ejsGetTrait(ejs->global, slotNum);
    ejsSetPropertyTrait(ejs, ejs->global, slotNum, ejs->typeType, EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED);

    /*
        Initialize the constructorf
        MOB - but what about native functions
     */
    constructor = (EjsFunction*) type;
    mprAssert(type->prototype);

#if UNUSED
    ejsDefineProperty(ejs, type->prototype, type->numInherited, ejsName(&qname, EJS_EJS_NAMESPACE, "constructor"), 
        ejs->functionType, EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED, (EjsObj*) fun);
#else
    code = &fun->body.code;
    ejsInitFunction(ejs, constructor, type->qname.name, code->byteCode, code->codeLen, fun->numArgs, fun->numDefault, 
        code->numHandlers, type, EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED, code->constants, NULL, fun->strict);
    constructor->activation = ejsClone(ejs, fun->activation, 0);
    constructor->thisObj = 0;
#endif

    //  MOB - what about the constructor property
    ejsDefineProperty(ejs, (EjsObj*) fun, -1, ejsName(&qname, EJS_EJS_NAMESPACE, "prototype"), ejs->objectType, 
        EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED, (EjsObj*) type->prototype);

    //  MOB -- this is currently only importing the "prototype" getter
    ejsBlendTypeProperties(ejs, type, ejs->typeType);
    return type;
}


/*
    Handcraft the Type and Object types upon which all other types depend.
 */
int ejsBootstrapTypes(Ejs *ejs)
{
    EjsType     *typeType, *objectType;
    EjsObj      *prototype, protostub;

    mprAssert(ejs);

    if ((typeType = createTypeVar(ejs, NULL, 0)) == 0 || (objectType = createTypeVar(ejs, NULL, 0)) == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    typeType->id = ES_Type;
    ejsName(&typeType->qname, "ejs", "Type");
    typeType->instanceSize = sizeof(EjsType);
    ejsSetDebugName(typeType, typeType->qname.name);
    ejs->typeType = typeType;

    objectType->id = ES_Object;
    ejsName(&objectType->qname, "ejs", "Object");
    objectType->instanceSize = sizeof(EjsObj);
    ejsSetDebugName(objectType, objectType->qname.name);
    ejs->objectType = objectType;

    ejsCreateObjectHelpers(ejs);
    ejsCloneObjectHelpers(ejs, typeType);
    
    memset(&protostub, 0, sizeof(protostub));
    objectType->prototype = &protostub;

    if ((prototype = ejsCreateObject(ejs, objectType, 0)) == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    ejsSetDebugName(prototype, "Type-Prototype");
    prototype->isPrototype = 1;
    typeType->prototype = prototype;

    if ((prototype = ejsCreateObject(ejs, objectType, 0)) == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    ejsSetDebugName(prototype, "Object-Prototype");
    prototype->isPrototype = 1;
    objectType->prototype = prototype;

    mprAddHash(ejs->coreTypes, typeType->qname.name, typeType);
    mprAddHash(ejs->coreTypes, objectType->qname.name, objectType);
    return 0;
}


/*
    Create a new type and initialize. BaseType is the super class for instances of the type being created. The
    returned EjsType will be an instance of EjsType. numTypeProp and  numInstanceProp should be set to the number
    of non-inherited properties.
 */
EjsType *ejsCreateType(Ejs *ejs, EjsName *qname, EjsModule *up, EjsType *baseType, EjsObj *prototype, 
        int instanceSize, int typeId, int numTypeProp, int numInstanceProp, int64 attributes, void *typeData)
{
    EjsType     *type;
    
    mprAssert(ejs);
    mprAssert(instanceSize > 0);
    
    type = createTypeVar(ejs, ejs->typeType, numTypeProp);
    if (type == 0) {
        return 0;
    }
    type->id = typeId;
    type->qname.name = qname->name;
    type->qname.space = qname->space;
    type->constructor.name = qname->name;
    type->module = up;
    type->typeData = typeData;
    type->baseType = baseType;
    type->instanceSize = instanceSize;
    setAttributes(type, attributes);
    ejsSetDebugName(type, type->qname.name);

    ejsCloneObjectHelpers(ejs, type);

    if (prototype) {
        type->prototype = prototype;
    } else {
        if ((type->prototype = ejsCreateObject(ejs, ejs->objectType, numInstanceProp)) == 0) {
            return 0;
        }
        ejsSetDebugName(type->prototype, mprStrcat(type, -1, type->qname.name, "-Prototype", NULL));
    }
    type->prototype->isPrototype = 1;

    if (baseType && ejsFixupType(ejs, type, baseType, 0) < 0) {
        return 0;
    }
    return type;
}


EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, EjsModule *up, EjsType *baseType, int numTypeProp, int numInstanceProp, 
    int64 attributes)
{
    type->module = up;
    setAttributes(type, attributes);

    if (numTypeProp > 0 && ejsGrowObject(ejs, &type->constructor.block.obj, numTypeProp) < 0) {
        return 0;
    }
    if (numInstanceProp > 0) {
        ejsGrowObject(ejs, type->prototype, numInstanceProp);
    }
    if (baseType && ejsFixupType(ejs, type, baseType, 0) < 0) {
        return 0;
    }
    return type;
}


/*
    OPT - should be able to just read in the attributes without having to stuff some in var and some in type.
    Should eliminate all the specific fields and just use BIT MASKS.
 */
static void setAttributes(EjsType *type, int64 attributes)
{
    if (attributes & EJS_TYPE_CALLS_SUPER) {
        type->callsSuper = 1;
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
        type->dynamicInstance = 1;
    }
    if (attributes & EJS_TYPE_FINAL) {
        type->final = 1;
    }
    if (attributes & EJS_TYPE_FIXUP) {
        type->needFixup = 1;
    }
    if (attributes & EJS_TYPE_HAS_CONSTRUCTOR) {
        type->hasConstructor = 1;
    }
    if (attributes & EJS_TYPE_HAS_INSTANCE_VARS) {
        type->hasInstanceVars = 1;
    }
    if (attributes & EJS_TYPE_IMMUTABLE) {
        type->immutable = 1;
    }
    if (attributes & EJS_TYPE_INTERFACE) {
        type->isInterface = 1;
    }
    if (attributes & EJS_TYPE_HAS_TYPE_INITIALIZER) {
        type->hasInitializer = 1;
    }
}


EjsType *ejsGetType(Ejs *ejs, int slotNum)
{
    EjsType     *type;

    if (slotNum < 0 || slotNum >= ejs->globalBlock->obj.numSlots) {
        return 0;
    }
    type = (EjsType*) ejsGetProperty(ejs, ejs->global, slotNum);
    if (type == 0 || !ejsIsType(type)) {
        return 0;
    }
    return type;
}


EjsType *ejsGetTypeByName(Ejs *ejs, cchar *space, cchar *name)
{
    EjsName     qname;

    return (EjsType*) ejsGetPropertyByName(ejs, ejs->global, ejsName(&qname, space, name));
}


static int inheritProperties(Ejs *ejs, EjsType *type, EjsObj *obj, int destOffset, EjsObj *baseBlock, int srcOffset, 
        int count, bool resetScope)
{
    EjsFunction     *fun;
    int             i;

    mprAssert(obj);
    mprAssert(baseBlock);
    mprAssert(count > 0);
    mprAssert(destOffset < obj->numSlots);
    mprAssert((destOffset + count) <= obj->numSlots);
    mprAssert(srcOffset < baseBlock->numSlots);
    mprAssert((srcOffset + count) <= baseBlock->numSlots);

    ejsCopySlots(ejs, obj, &obj->slots[destOffset], &baseBlock->slots[srcOffset], count, 1);
    
    if (resetScope) {
        for (i = destOffset; i < (destOffset + count); i++) {
            fun = (EjsFunction*) ejsGetProperty(ejs, obj, i);
            if (ejsIsFunction(fun)) {
                fun = ejsCloneFunction(ejs, fun, 0);
                ejsSetProperty(ejs, obj, i, (EjsObj*) fun);
                fun->thisObj = 0;
                fun->block.scope = (EjsBlock*) type;
            }
        }
    }
    ejsMakeObjHash(obj);
    return 0;
}


static void fixInstanceSize(Ejs *ejs, EjsType *type)
{
    EjsType     *tp;

    for (tp = type->baseType; tp && tp != ejs->objectType; tp = tp->baseType) {
        if (tp->instanceSize > type->instanceSize) {
            type->instanceSize = tp->instanceSize;
        }
    }
}


/*
    Fixup a type. This is used by the compiler and loader when it must first define a type when its base type or
    property types may not yet be defined (ie. forward references. Consequently, it must fixup the type and its 
    counts of inherited properties. It must also copy inherited slots and traits.
 */
int ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom)
{
    mprAssert(ejs);
    mprAssert(type);
    mprAssert(type != baseType);

    type->needFixup = 0;
    type->baseType = baseType;
    
    if (baseType) {
        //  MOB -- should be able to remove the || baseType->hasBaseConstructors
        if (baseType->hasConstructor || baseType->hasBaseConstructors) {
            type->hasBaseConstructors = 1;
        }
        //  MOB -- when compiling baseType is always != ejs->objectType
        //  MOB _- should not explicity reference objecttype
        if (baseType != ejs->objectType && baseType->dynamicInstance) {
            type->dynamicInstance = 1;
        }
        type->hasInstanceVars |= baseType->hasInstanceVars;
    }
    if (type->implements) {
        if (fixupTypeImplements(ejs, type, makeRoom) < 0) {
            return EJS_ERR;
        }
    }
    if (baseType) {
        if (type->implements || baseType->prototype->numSlots > 0) {
            fixupPrototypeProperties(ejs, type, baseType, makeRoom);
        }
    }
    fixInstanceSize(ejs, type);
    return 0;
}


/*
    Import properties from the Type class. These are not inherited in the usual sense and numInherited is not updated.
    The properties are directly copied.
 */
int ejsBlendTypeProperties(Ejs *ejs, EjsType *type, EjsType *typeType)
{
    int     count, destOffset, srcOffset;

    mprAssert(type);
    mprAssert(typeType);

    count = ejsGetPropertyCount(ejs, (EjsObj*) typeType) - typeType->numInherited;
    if (count > 0) { 
        /*  Append properties to the end of the type so as to not mess up the first slot which may be an initializer */
        destOffset = ejsGetPropertyCount(ejs, (EjsObj*) type);
        srcOffset = 0;
        if (ejsGrowObject(ejs, (EjsObj*) type, type->constructor.block.obj.numSlots + count) < 0) {
            return EJS_ERR;
        }
        if (inheritProperties(ejs, type, (EjsObj*) type, destOffset, (EjsObj*) typeType, srcOffset, count, 0) < 0) {
            return EJS_ERR;
        }
    }
#if FUTURE && KEEP
    protoCount = ejsGetPropertyCount(ejs, typeType->prototype);
    if (protoCount > 0) {
        srcOffset = typeType->numInherited;
        destOffset = ejsGetPropertyCount(ejs, type->prototype);
        if (ejsGrowObject(ejs, (EjsObj*) type->prototype, type->prototype->numSlots + protoCount) < 0) {
            return EJS_ERR;
        }
        if (inheritProperties(ejs, type, (EjsObj*) type->prototype, destOffset, (EjsObj*) typeType->prototype, srcOffset, 
                protoCount, 0) < 0) {
            return EJS_ERR;
        }
    }
#endif
    return 0;
}


static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom)
{
    EjsType         *iface;
    EjsBlock        *bp;
    EjsNamespace    *nsp;
    int             next, offset, count, nextNsp;

    mprAssert(type);
    mprAssert(type->implements);

    offset = type->constructor.block.obj.numSlots;
    if (makeRoom) {
        count = 0;
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (!iface->isInterface) {
                count += iface->constructor.block.obj.numSlots;
                type->hasInstanceVars |= iface->hasInstanceVars;
            }
        }
        if (count > 0 && ejsInsertGrowObject(ejs, (EjsObj*) type, count, 0) < 0) {
            return EJS_ERR;
        }
    }
    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (!iface->isInterface) {
            count = iface->constructor.block.obj.numSlots;
            if (inheritProperties(ejs, type, (EjsObj*) type, offset, (EjsObj*) iface, 0, count, 1) < 0) {
                return EJS_ERR;
            }
            offset += count;
            for (nextNsp = 0; (nsp = (EjsNamespace*) ejsGetNextItem(&iface->constructor.block.namespaces, &nextNsp)) != 0;) {
                ejsAddNamespaceToBlock(ejs, (EjsBlock*) type, nsp);
            }
            for (bp = iface->constructor.block.scope; bp->scope; bp = bp->scope) {
                for (nextNsp = 0; (nsp = (EjsNamespace*) ejsGetNextItem(&bp->namespaces, &nextNsp)) != 0;) {
                    ejsAddNamespaceToBlock(ejs, (EjsBlock*) type, nsp);
                }
            }
        }
    }
    return 0;
}


static int fixupPrototypeProperties(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom)
{
    EjsType     *iface;
    EjsObj      *basePrototype;
    int         count, offset, next;

    mprAssert(type != baseType);
    mprAssert(type->prototype);
    mprAssert(baseType->prototype);
    
    basePrototype = baseType->prototype;

    if (makeRoom) {
        count = 0;
        mprAssert(basePrototype);
        /* Must inherit if the type has instance vars */
        if (basePrototype && type->hasInstanceVars) {
            count = basePrototype->numSlots;
        }
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (!iface->isInterface && iface->prototype) {
                count += iface->prototype->numSlots - iface->numInherited;
            }
        }
        if (count > 0 && ejsInsertGrowObject(ejs, type->prototype, count, 0) < 0) {
            return EJS_ERR;
        }
    }
    offset = 0;
    if (type->hasInstanceVars) {
        mprAssert(type->prototype->numSlots >= basePrototype->numSlots);
        if (inheritProperties(ejs, type, type->prototype, offset, basePrototype, 0, basePrototype->numSlots, 0) < 0) {
            return EJS_ERR;
        }
        type->numInherited = basePrototype->numSlots;
        offset += basePrototype->numSlots;
    }

    if (type->implements) {
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (iface->isInterface) {
                continue;
            }
            /* Only come here for implemented classes */
            if (iface->prototype == 0) {
                continue;
            }
            count = iface->prototype->numSlots - iface->numInherited;
            if (inheritProperties(ejs, type, type->prototype, offset, iface->prototype, iface->numInherited, count, 1) < 0) {
                return EJS_ERR;
            }
            type->numInherited += count;
            offset += count;
        }
    }
    return 0;
}


/*
    Set the native method function for a function property
 */
int ejsBindMethod(Ejs *ejs, void *obj, int slotNum, EjsProc nativeProc)
{
    return ejsBindFunction(ejs, obj, slotNum, nativeProc);
}


int ejsBindAccess(Ejs *ejs, void *obj, int slotNum, EjsProc getter, EjsProc setter)
{
    EjsFunction     *fun;

    if (getter) {
        if (ejsBindFunction(ejs, obj, slotNum, getter) < 0) {
            return EJS_ERR;
        }
    }
    if (setter) {
        fun = (EjsFunction*) ejsGetProperty(ejs, obj, slotNum);
        if (fun == 0 || !ejsIsFunction(fun) || fun->setter == 0 || !ejsIsFunction(fun->setter)) {
            ejs->hasError = 1;
            mprError(ejs, "Attempt to bind non-existant setter function for slot %d in \"%s\"", slotNum, 
                ejsGetDebugName(obj));
            return EJS_ERR;
        }
        fun = fun->setter;
        if (fun->body.code.codeLen != 0) {
            mprError(ejs, "Setting a native method on a non-native function \"%s\" in \"%s\"", fun->name, 
                ejsGetDebugName(obj));
            ejs->hasError = 1;
        }
        fun->body.proc = setter;
        fun->isNativeProc = 1;
    }
    return 0;
}


/*
    Set the native method function for a function property
 */
int ejsBindFunction(Ejs *ejs, void *obj, int slotNum, EjsProc nativeProc)
{
    EjsFunction     *fun;

    if (ejsGetPropertyCount(ejs, obj) < slotNum) {
        ejs->hasError = 1;
        mprError(ejs, "Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, ejsGetDebugName(obj));
        return EJS_ERR;
    }
    fun = (EjsFunction*) ejsGetProperty(ejs, obj, slotNum);
    if (fun == 0 || !ejsIsFunction(fun)) {
        mprAssert(fun);
        ejs->hasError = 1;
        mprError(ejs, "Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, ejsGetDebugName(obj));
        return EJS_ERR;
    }
    if (fun->body.code.codeLen != 0) {
        mprError(ejs, "Setting a native method on a non-native function \"%s\" in \"%s\"", fun->name, ejsGetDebugName(obj));
        ejs->hasError = 1;
    }
    mprAssert(fun->body.proc == 0);
    fun->body.proc = nativeProc;
    fun->isNativeProc = 1;
    return 0;
}


void ejsBindConstructor(Ejs *ejs, EjsType *type, EjsProc nativeProc)
{
    mprAssert(type->hasConstructor);
    mprAssert(type->constructor.isConstructor);
    mprAssert(type->constructor.block.obj.isFunction);
    mprAssert(type->constructor.body.proc == 0);
    type->constructor.body.proc = nativeProc;
    type->constructor.isNativeProc = 1;
}


/*
    Define a global public function. Returns a positive slot number, otherwise a negative MPR error.
 */
int ejsDefineGlobalFunction(Ejs *ejs, cchar *name, EjsProc fn)
{
    EjsFunction *fun;
    EjsName     qname;

    if ((fun = ejsCreateFunction(ejs, name, NULL, -1, 0, 0, 0, ejs->objectType, 0, NULL, NULL, 0)) == 0) {
        return MPR_ERR_NO_MEMORY;
    }
    fun->body.proc = fn;
    fun->isNativeProc = 1;
    ejsName(&qname, EJS_PUBLIC_NAMESPACE, name);
    return ejsSetPropertyByName(ejs, ejs->global, &qname, (EjsObj*) fun);
}


/*
    Return true if target is an instance of type or a sub class of it.
 */
bool ejsIsA(Ejs *ejs, EjsObj *target, EjsType *type)
{
    mprAssert(type);

    if (!ejsIsType(type) || target == 0) {
        return 0;
    }
    return ejsIsTypeSubType(ejs, target->type, type);
}


/*
    Return true if "target" is a "type", subclass of "type" or implements "type".
 */
bool ejsIsTypeSubType(Ejs *ejs, EjsType *target, EjsType *type)
{
    EjsType     *tp, *iface;
    int         next;

    mprAssert(target);
    mprAssert(type);
    
    if (!ejsIsType(target) || !ejsIsType(type)) {
        return 0;
    }
    /*
        See if target is a subtype of type
     */
    for (tp = target; tp; tp = tp->baseType) {
        /*
            Test ID also to allow cloned interpreters to match where the IDs are equal
         */
        if (tp == type || tp->id == type->id) {
            return 1;
        }
    }
    /*
        See if target implements type
     */
    if (target->implements) {
        for (next = 0; (iface = mprGetNextItem(target->implements, &next)) != 0; ) {
            if (iface == type) {
                return 1;
            }
        }
    }
    return 0;
}


/*
    Get the attributes of the type property at slotNum.

 */
int ejsGetTypePropertyAttributes(Ejs *ejs, EjsObj *vp, int slotNum)
{
    EjsType     *type;

    if (!ejsIsType(vp)) {
        mprAssert(ejsIsType(vp));
        return EJS_ERR;
    }
    type = (EjsType*) vp;
    return ejsGetTraitAttributes((EjsObj*) type, slotNum);
}


/*
    This call is currently only used to update the type namespace after resolving a run-time namespace.
 */
void ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName *qname)
{
    type->qname.name = qname->name;
    type->qname.space = qname->space;
    ejsSetDebugName(type, qname->name);

    if (type->prototype) {
        ejsSetDebugName(type->prototype, qname->name);
    }
}


/*
    Define namespaces for a class. Inherit the protected and internal namespaces from all base classes.
 */
void ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type)
{
    EjsNamespace        *nsp;

    if (type->baseType) {
        /*
            Inherit the base class's protected and internal namespaces
         */
        ejsInheritBaseClassNamespaces(ejs, type, type->baseType);
    }
    //  TODO - add readonly here
    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PROTECTED_NAMESPACE);
    nsp->flags |= EJS_NSP_PROTECTED;
    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PRIVATE_NAMESPACE);
    nsp->flags |= EJS_NSP_PRIVATE;
}


/*
    Return the total memory size used by a type
    MOB - not counting name sizes or methods etc. Should count everything.
 */
static int ejsGetBlockSize(Ejs *ejs, EjsObj *obj)
{
    int     size, numSlots;

    numSlots = ejsGetPropertyCount(ejs, obj);
    size = (numSlots * sizeof(EjsSlot));
    if (obj->hash) {
        size += sizeof(EjsHash) + (obj->hash->size * sizeof(int));
    }
    if (ejsIsBlock(obj)) {
        size += sizeof(EjsBlock) - sizeof(EjsObj);
    }
    if (ejsIsType(obj)) {
        size += sizeof(EjsType) + sizeof(EjsTypeHelpers);
        size += sizeof(EjsType) - sizeof(EjsBlock);
    }
    return size;
}


/*
    Return the total memory size used by a type
 */
int ejsGetTypeSize(Ejs *ejs, EjsType *type)
{
    int     size;

    size = ejsGetBlockSize(ejs, (EjsObj*) type);
    if (type->prototype) {
        size += ejsGetBlockSize(ejs, type->prototype);
    }
    return size;
}


/*********************************** Methods **********************************/
/*
    function get prototype(): Object
 */
static EjsObj *type_prototype(Ejs *ejs, EjsType *type, int argc, EjsObj **argv)
{
    mprAssert(ejsIsType(type));

    return type->prototype;
}


/*********************************** Factory **********************************/

void ejsCreateTypeType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->typeType;

    ejsCloneBlockHelpers(ejs, type);

    type->helpers.clone            = (EjsCloneHelper) cloneTypeVar;
    type->helpers.create           = (EjsCreateHelper) createTypeVar;
    type->helpers.lookupProperty   = (EjsLookupPropertyHelper) lookupTypeProperty;
    type->helpers.setProperty      = (EjsSetPropertyHelper) setTypeProperty;
    type->helpers.mark             = (EjsMarkHelper) markType;

    /*
        WARNING: read closely. This can be confusing. Fixup the helpers for the object type. We need to find
        helpers via objectType->type->helpers. So we set it to the Type type. We keep objectType->baseType == 0
        because Object has no base type. Similarly for the Type type.
     */
    ejs->objectType->constructor.block.obj.type = ejs->typeType;
    ejs->blockType->constructor.block.obj.type = ejs->typeType;
    ejs->typeType->constructor.block.obj.type = ejs->objectType;
}


void ejsCompleteType(Ejs *ejs, EjsType *type)
{
    EjsName     qname;
    EjsFunction *fun;
    int         slotNum;
    
    slotNum = ejsLookupProperty(ejs, (EjsObj*) type, ejsName(&qname, EJS_EJS_NAMESPACE, "prototype"));
    if (slotNum >= 0) {
        /* Prototype is not defined native so that modules don't get marked as requiring native libraries */
        fun = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) type, slotNum);
        fun->body.code.codeLen = 0;
        fun->body.proc = 0;
        ejsBindMethod(ejs, type, slotNum, (EjsProc) type_prototype);
    }
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
