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
static int inheritProperties(Ejs *ejs, EjsObj *obj, int offset, EjsObj *baseBlock, int count, bool implementing);
static void setAttributes(EjsType *type, int attributes);

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
    dest->hasBaseConstructors = src->hasBaseConstructors;
    dest->hasBaseInitializers = src->hasBaseInitializers;
    dest->hasBaseStaticInitializers = src->hasBaseStaticInitializers;
    dest->hasConstructor = src->hasConstructor;
    dest->hasInitializer = src->hasInitializer;
mprAssert(dest->hasConstructor == dest->hasInitializer);
    dest->hasMeta = src->hasMeta;
    dest->hasStaticInitializer = src->hasStaticInitializer;
    dest->helpers = src->helpers;
    dest->id = src->id;
    dest->immutable = src->immutable;
    dest->initialized = src->initialized;
    dest->prototype = src->prototype;
    dest->instanceSize = src->instanceSize;
    dest->isInterface = src->isInterface;
    dest->module = src->module;
    dest->numericIndicies = src->numericIndicies;
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
            typeSize += (sizeHash * (int) sizeof(EjsSlot*));
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
    ejsInitList(&type->block.namespaces);

    if (!dynamic) {
        /*
            This is for a fixed type. This is the normal case when not compiling. Layout is:

            Slots       sizeof(EjsSlot) * numSlots
            Hash        ejsGetHashSize(numslots)
         */
        start = (char*) type + sizeof(EjsType);

        if (numSlots > 0) {
            obj->sizeSlots = numSlots;
            obj->slots = (EjsSlot*) start;
            ejsZeroSlots(ejs, obj->slots, numSlots);
            start += sizeof(EjsSlot) * numSlots;
        }
        if (sizeHash > 0) {
            obj->hash = (int*) start;
            obj->sizeHash = sizeHash;
            memset(obj->hash, -1, sizeHash * sizeof(int));
            start += sizeof(int) * sizeHash;
        }
        mprAssert((start - (char*) type) <= typeSize);
    }
    ejsSetDebugName(vp, "type");
#if BLD_DEBUG
    ejsAddToGcStats(ejs, vp, ES_Type);
#endif
    return type;
}


/*
    Lookup a property with a namespace qualifier in an object and return the slot if found. Return EJS_ERR if not found.
 */
static int lookupTypeProperty(struct Ejs *ejs, EjsType *type, EjsName *qname)
{
    EjsName     name;
    int         slotNum;

    slotNum = (ejs->objectType->helpers.lookupProperty)(ejs, (EjsObj*) type, qname);

    if (slotNum < 0 && strcmp(qname->name, "prototype") == 0 && qname->space[0] == '\0') {
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
    if (slotNum < 0 && !type->block.obj.dynamic) {
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
    int numInstanceProp, int attributes)
{
    EjsType     *type;

    type = ejsCreateType(ejs, qname, NULL, baseType, instanceSize, id, numTypeProp, numInstanceProp, attributes, 0);
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


EjsType *ejsCreateTypeFromFunction(Ejs *ejs, EjsFunction *fun)
{
    EjsName     qname;
    EjsType     *type;
    EjsTrait    *trait;
    int         slotNum;

    slotNum = ejsGetPropertyCount(ejs, ejs->global);

    type = ejsCreateType(ejs, ejsName(&qname, fun->name, "-prototype-"), NULL, ejs->objectType, ejs->objectType->instanceSize,
        slotNum, ES_Object_NUM_CLASS_PROP, ES_Object_NUM_INSTANCE_PROP, 
        EJS_TYPE_DYNAMIC_INSTANCE | EJS_TYPE_HAS_CONSTRUCTOR, NULL);
    if (type == 0) {
        return 0;
    }
    type->dontCopyPrototype = 1;

    /*
        Type is installed, but not hashed in the global names
     */
    ejsSetProperty(ejs, ejs->global, slotNum, (EjsObj*) type);
    trait = ejsGetTrait(ejs->global, slotNum);
    ejsSetPropertyTrait(ejs, ejs->global, slotNum, ejs->typeType, EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED);

    /*
        Install the function as the constructor
     */
    mprAssert(type->prototype);
    ejsSetProperty(ejs, (EjsObj*) type->prototype, type->numPrototypeInherited, (EjsObj*) fun);
    ejsSetPropertyTrait(ejs, type->prototype, type->numPrototypeInherited, ejs->functionType, 
        EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED);
    fun->constructor = 1;
    fun->thisObj = 0;
    return type;
}


/*
    Handcraft the Type and Object types upon which all other types depend.
 */
int ejsBootstrapTypes(Ejs *ejs)
{
    EjsType     *typeType, *objectType;
    EjsObj      *prototype;

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
EjsType *ejsCreateType(Ejs *ejs, EjsName *qname, EjsModule *up, EjsType *baseType, int instanceSize, int typeId, 
        int numTypeProp, int numInstanceProp, int attributes, void *typeData)
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
    type->module = up;
    type->typeData = typeData;
    type->baseType = baseType;
    type->instanceSize = instanceSize;
    setAttributes(type, attributes);
    ejsSetDebugName(type, type->qname.name);

    ejsCloneObjectHelpers(ejs, type);

    if ((type->prototype = ejsCreateObject(ejs, ejs->objectType, numInstanceProp)) == 0) {
        return 0;
    }
    ejsSetDebugName(type->prototype, mprStrcat(type, -1, type->qname.name, "-Prototype", NULL));
    type->prototype->isPrototype = 1;

    if (baseType && ejsFixupType(ejs, type, baseType, 0) < 0) {
        return 0;
    }
    return type;
}


EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, EjsModule *up, EjsType *baseType, int numTypeProp, int numInstanceProp, 
    int attributes)
{
    type->module = up;
    setAttributes(type, attributes);

    if (numTypeProp > 0 && ejsGrowObject(ejs, &type->block.obj, numTypeProp) < 0) {
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
static void setAttributes(EjsType *type, int attributes)
{
    if (attributes & EJS_TYPE_FINAL) {
        type->final = 1;
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCE) {
        type->dynamicInstance = 1;
    }
    if (attributes & EJS_TYPE_HAS_CONSTRUCTOR) {
        type->hasConstructor = 1;
    }
    if (attributes & EJS_TYPE_HAS_INITIALIZER) {
        type->hasInitializer = 1;
    }
mprAssert(type->hasConstructor == type->hasInitializer);
    if (attributes & EJS_TYPE_IMMUTABLE) {
        type->immutable = 1;
    }
    if (attributes & EJS_TYPE_INTERFACE) {
        type->isInterface = 1;
    }
    if (attributes & EJS_TYPE_FIXUP) {
        type->needFixup = 1;
    }
    if (attributes & EJS_TYPE_HAS_TYPE_INITIALIZER) {
        type->hasStaticInitializer = 1;
    }
    if (attributes & EJS_TYPE_CALLS_SUPER) {
        type->callsSuper = 1;
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


static int inheritProperties(Ejs *ejs, EjsObj *obj, int offset, EjsObj *baseBlock, int count, bool implementing)
{
    int     start;

    mprAssert(obj);
    mprAssert(baseBlock);
    mprAssert(count > 0);

    if (baseBlock == 0 || count <= 0) {
        return 0;
    }
    start = baseBlock->numSlots - count;
    ejsCopySlots(ejs, obj, &obj->slots[offset], &baseBlock->slots[start], count, 1);
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
        if (baseType->hasConstructor || baseType->hasBaseConstructors) {
            type->hasBaseConstructors = 1;
        }
        if (baseType->hasInitializer || baseType->hasBaseInitializers) {
            type->hasBaseInitializers = 1;
        }
mprAssert(type->hasBaseConstructors == type->hasBaseInitializers);
        if (baseType != ejs->objectType && baseType->dynamicInstance) {
            type->dynamicInstance = 1;
        }
    }
    if (type->implements) {
        if (fixupTypeImplements(ejs, type, makeRoom) < 0) {
            return EJS_ERR;
        }
    }
    if (baseType) {
        if (type->implements || baseType->prototype->numSlots > 0) {
            //  MOB -- if Object has instance methods, then every type MUST have a prototype
            mprAssert(type->baseType == baseType);
            fixupPrototypeProperties(ejs, type, baseType, makeRoom);
        }
    }
    fixInstanceSize(ejs, type);
    return 0;
}


static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom)
{
    EjsType         *iface;
    EjsNamespace    *nsp;
    int             next, offset, count, nextNsp;

    mprAssert(type);
    mprAssert(type->implements);

    if (makeRoom) {
        count = 0;
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (!iface->isInterface) {
                count += iface->block.obj.numSlots;
            }
        }
        if (count > 0 && ejsInsertGrowObject(ejs, (EjsObj*) type, count, 0) < 0) {
            return EJS_ERR;
        }
    }
    offset = 0;
    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (!iface->isInterface) {
            count = iface->block.obj.numSlots;
            if (inheritProperties(ejs, (EjsObj*) type, offset, (EjsObj*) iface, count, 1) < 0) {
                return EJS_ERR;
            }
        }
        for (nextNsp = 0; (nsp = (EjsNamespace*) ejsGetNextItem(&iface->block.namespaces, &nextNsp)) != 0;) {
            mprAssert(ejsIsBlock(type));
            ejsAddNamespaceToBlock(ejs, (EjsBlock*) type, nsp);
        }
    }
    return 0;
}


static int fixupPrototypeProperties(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom)
{
    EjsType     *iface;
    EjsObj      *ip, *basePrototype;
    int         count, offset, next;

    mprAssert(type != baseType);
    mprAssert(type->prototype);
    mprAssert(baseType->prototype);
    
    basePrototype = baseType->prototype;

    if (makeRoom) {
        count = 0;
        if (basePrototype) {
            count = basePrototype->numSlots;
        }
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (!iface->isInterface && iface->prototype) {
                count += iface->prototype->numSlots;
            }
        }
        if (count > 0 && ejsInsertGrowObject(ejs, type->prototype, count, 0) < 0) {
            return EJS_ERR;
        }
    }
    offset = 0;
    if (inheritProperties(ejs, type->prototype, offset, basePrototype, basePrototype->numSlots, 0) < 0) {
        return EJS_ERR;
    }
    type->numPrototypeInherited = basePrototype->numSlots;
    offset += type->prototype->numSlots;

    if (type->implements) {
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (iface->isInterface) {
                continue;
            }
            /* Only come here for implemented classes */
            if ((ip = iface->prototype) == 0) {
                continue;
            }
            count = ip->numSlots;
            if (inheritProperties(ejs, type->prototype, offset, ip, count, 1) < 0) {
                return EJS_ERR;
            }
            offset += ip->numSlots;
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
        fun->nativeProc = 1;
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
    fun->body.proc = nativeProc;
    fun->nativeProc = 1;
    return 0;
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
    fun->nativeProc = 1;
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
    size += (obj->sizeHash * sizeof(int));
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
    ejs->objectType->block.obj.type = ejs->typeType;
    ejs->blockType->block.obj.type = ejs->typeType;
    ejs->typeType->block.obj.type = ejs->objectType;
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
