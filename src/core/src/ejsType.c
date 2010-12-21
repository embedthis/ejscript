/**
    ejsType.c - Type class

    The type class is the base class for all types (classes) in the system.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static int defaultManager(EjsObj *ev, int flags);
static void fixInstanceSize(Ejs *ejs, EjsType *type);
static int fixupPrototypeProperties(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom);
static int inheritProperties(Ejs *ejs, EjsType *type, EjsPot *obj, int destOffset, EjsPot *baseBlock, int srcOffset, 
    int count, bool resetScope);
static void manageType(EjsType *type, int flags);
static void setAttributes(EjsType *type, int64 attributes);

/******************************************************************************/
/*
    Copy a type. 

    function copy(type: Object): Object
 */
//  MOB -- is this ever used?
static EjsType *cloneTypeVar(Ejs *ejs, EjsType *src, bool deep)
{
    EjsType     *dest;

    if (! ejsIsType(ejs, src)) {
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
    Create a new Type object. numProp is the number of property slots to pre-allocate.
 */
static EjsType *createTypeVar(Ejs *ejs, EjsType *typeType, int numProp)
{
    EjsType     *type;
    EjsPot      *obj;
    char        *start;
    int         typeSize, sizeHash, dynamic;

    mprAssert(ejs);
    
    /*
        If the compiler is building itself (empty mode), then the types themselves must be dynamic. Otherwise, the type
        is fixed and will contain the names hash and traits in one memory block. 
        NOTE: don't confuse this with dynamic objects.
     */
    sizeHash = 0;
    if (numProp < 0 || ejs->empty || ejs->flags & EJS_FLAG_DYNAMIC) {
        dynamic = 1;
        typeSize = sizeof(EjsType);
        numProp = 0;
    } else {
        dynamic = 0;
        typeSize = sizeof(EjsType) + sizeof(EjsProperties);
        typeSize += (int) sizeof(EjsSlot) * numProp;
        if (numProp > EJS_HASH_MIN_PROP) {
            sizeHash = ejsGetHashSize(numProp);
            typeSize += sizeof(EjsHash) + (sizeHash * (int) sizeof(EjsSlot*));
        }
    }
    if ((type = mprAllocBlock(typeSize, MPR_ALLOC_ZERO | MPR_ALLOC_MANAGER)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    mprSetManager(type, manageType);
    mprInitList(&type->constructor.block.namespaces);
    type->ejs = ejs;
    obj = (EjsPot*) type;
    TYPE(obj) = typeType;
    DYNAMIC(obj) = dynamic;
    obj->isType = 1;
    obj->isBlock = 1;
    ejsSetMemRef(obj);

    if (!dynamic) {
        /*
            This is for a fixed type. This is the normal case when not compiling. Layout is:
                Slots: sizeof(EjsSlot) * numProp
                Hash:  ejsGetHashSize(numslots)
         */
        start = (char*) type + sizeof(EjsType);
        if (numProp > 0) {
            obj->properties = (EjsProperties*) start;
            obj->properties->size = numProp;
            ejsZeroSlots(ejs, obj->properties->slots, numProp);
            start += sizeof(EjsProperties) + sizeof(EjsSlot) * numProp;
        }
        if (sizeHash > 0) {
            obj->properties->hash = (EjsHash*) start;
            obj->properties->hash->buckets = (int*) (start + sizeof(EjsHash));
            obj->properties->hash->size = sizeHash;
            memset(obj->properties->hash->buckets, -1, sizeHash * sizeof(int));
            start += sizeof(EjsHash) + sizeof(int) * sizeHash;
        }
        mprAssert((start - (char*) type) <= typeSize);
    }
    return type;
}


static int setTypeProperty(Ejs *ejs, EjsType *type, int slotNum, EjsObj *value)
{
    if (slotNum < 0 && !DYNAMIC(type)) {
        ejsThrowTypeError(ejs, "Object is not dynamic");
        return EJS_ERR;
    }
    return (ejs->blockType->helpers.setProperty)(ejs, (EjsObj*) type, slotNum, value);
}


/******************************** Native Type API *****************************/

static EjsType *createBootType(Ejs *ejs, int id, int size, int dynamic, void *manager)
{
    EjsType     *type;

    if ((type = createTypeVar(ejs, NULL, 0)) == NULL) {
        return NULL;
    }
    /*
        MOB - try to remove id. It is used in cast helpers
     */
    type->id = id;
    type->instanceSize = size;
    type->dynamicInstance = dynamic;
    type->manager = manager;
    type->ejs = ejs;
    return type;
}


static void createBootPrototype(Ejs *ejs, EjsType *type, cchar *name)
{
    type->qname = N("ejs", name);
    if ((type->prototype = ejsCreatePot(ejs, ejs->objectType, 0)) == 0) {
        return;
    }
    type->prototype->isPrototype = 1;
    ejsSetName(type, name);
    ejsSetName(type->prototype, name);
    ejsSetPropertyByName(ejs, ejs->coreTypes, type->qname, type);
}

/*
    Handcraft the Array, Object, String and Type classes.
 */
int ejsBootstrapTypes(Ejs *ejs)
{
    EjsPot  protostub;

    mprAssert(ejs);

    ejsCreateObjHelpers(ejs);
    ejsCreatePotHelpers(ejs);

    ejs->stringType = createBootType(ejs, ES_String, sizeof(EjsString), 0, ejsManageString);
    ejs->typeType   = createBootType(ejs, ES_Type, sizeof(EjsType), 1, manageType);
    ejs->objectType = createBootType(ejs, ES_Object, sizeof(EjsPot), 1, ejsManagePot);

    ejs->typeType->isPot = 1;
    ejs->objectType->isPot = 1;

    ejsInitStringType(ejs, ejs->stringType);
    ejsClonePotHelpers(ejs, ejs->typeType);
    ejsClonePotHelpers(ejs, ejs->objectType);

    memset(&protostub, 0, sizeof(protostub));
    ejs->objectType->prototype = &protostub;

    ejs->coreTypes = ejsCreateEmptyPot(ejs);
    ejsSetName(ejs->coreTypes, "coreTypes");
    createBootPrototype(ejs, ejs->typeType, "Type");
    createBootPrototype(ejs, ejs->objectType, "Object");
    createBootPrototype(ejs, ejs->stringType, "String");
    return 0;
}


static int defaultManager(EjsObj *ev, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
#if UNUSED
        //  MOB -- should not do this
            mprMark(ev->type);
        }
#endif
    }
    return 0;
}


/*
    Create a new type and initialize. BaseType is the super class for instances of the type being created. The
    returned EjsType will be an instance of EjsType. numTypeProp and numInstanceProp should be set to the number
    of non-inherited properties.
 */
EjsType *ejsCreateType(Ejs *ejs, EjsName qname, EjsModule *up, EjsType *baseType, EjsPot *prototype, int instanceSize, 
        int typeId, int numTypeProp, int numInstanceProp, int64 attributes)
{
    EjsType     *type;
    
    mprAssert(ejs);
    mprAssert(instanceSize >= 0);
    
    type = createTypeVar(ejs, ejs->typeType, numTypeProp);
    if (type == 0) {
        return 0;
    }
    type->manager = ejsManagePot;
    type->id = typeId;
    type->qname = qname;
    type->constructor.name = qname.name;
    type->module = up;
    type->baseType = baseType;
    type->instanceSize = instanceSize;
    setAttributes(type, attributes);
    ejsSetName(type, MPR_NAME("type"));

    if (prototype) {
        type->prototype = prototype;
    } else {
        if ((type->prototype = ejsCreatePot(ejs, ejs->objectType, numInstanceProp)) == 0) {
            return 0;
        }
        ejsSetName(type->prototype, MPR_NAME("type"));
    }
    type->prototype->isPrototype = 1;

    if (baseType && ejsFixupType(ejs, type, baseType, 0) < 0) {
        return 0;
    }
    return type;
}


/*
    Create a native built-in type. This is used for the core native classes of the language.
 */
EjsType *ejsCreateNativeType(Ejs *ejs, EjsName qname, int id, int instanceSize, void *manager, int helpers)
{
    EjsType     *type;

    type = ejsCreateType(ejs, qname, NULL, NULL, NULL, instanceSize, id, 0, 0, 0);
    if (type == 0) {
        ejs->hasError = 1;
        return 0;
    }
    ejsSetPropertyByName(ejs, ejs->coreTypes, type->qname, type);
    type->manager = manager ? manager : defaultManager;
    if (helpers == EJS_POT_HELPERS) {
        ejsClonePotHelpers(ejs, type);
    } else if (helpers == EJS_OBJ_HELPERS) {
        ejsCloneObjHelpers(ejs, type);
    }
    return type;
}


EjsType *ejsConfigureNativeType(Ejs *ejs, EjsName qname, int instanceSize, void *manager, int helpers)
{
    EjsType     *type;

    if ((type = ejsGetTypeByName(ejs, qname)) == 0) {
        mprError("Can't find %N type", qname);
        return 0;
    }
    type->instanceSize = instanceSize;
    type->manager = manager ? manager : defaultManager;
    if (helpers == EJS_POT_HELPERS) {
        ejsClonePotHelpers(ejs, type);
        type->isPot = 1;
    } else if (helpers == EJS_OBJ_HELPERS) {
        ejsCloneObjHelpers(ejs, type);
        type->isPot = 0;
    }
    return type;
}


EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, EjsModule *up, EjsType *baseType, int numTypeProp, int numInstanceProp, 
    int64 attributes)
{
    type->module = up;
    setAttributes(type, attributes);

    if (numTypeProp > 0 && ejsGrowPot(ejs, &type->constructor.block.pot, numTypeProp) < 0) {
        return 0;
    }
    if (numInstanceProp > 0) {
        ejsGrowPot(ejs, type->prototype, numInstanceProp);
    }
    if (baseType && ejsFixupType(ejs, type, baseType, 0) < 0) {
        return 0;
    }
    return type;
}


EjsType *ejsCreateArchetype(Ejs *ejs, EjsFunction *fun, EjsPot *prototype)
{
    EjsName     qname;
    EjsType     *type, *baseType;
    EjsCode     *code;
    EjsString   *name;

    //  MOB -- get rid of type ids
    static int  id = 10000;

    if (prototype == 0 && fun) {
        prototype = ejsGetPropertyByName(ejs, (EjsObj*) fun, N(NULL, "prototype"));
    }
    baseType = prototype ? TYPE(prototype): ejs->objectType;
    name = (fun && fun->name) ? fun->name : ejsCreateStringFromAsc(ejs, "-type-from-function-");
    qname.space = ejsCreateStringFromAsc(ejs, EJS_PROTOTYPE_NAMESPACE);
    qname.name = name;
    type = ejsCreateType(ejs, qname, NULL, baseType, prototype, ejs->objectType->instanceSize, id++, 0, 0, 
        EJS_TYPE_DYNAMIC_INSTANCE);
    if (type == 0) {
        return 0;
    }
    if (fun) {
        code = fun->body.code;
        /*  MOB -- using ejs->objectType as the return type because the Yahoo module pattern returns {} in the constructor */
        ejsInitFunction(ejs, (EjsFunction*) type, type->qname.name, code->byteCode, code->codeLen, 
            fun->numArgs, fun->numDefault, code->numHandlers, ejs->objectType, EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED, 
            code->module, NULL, fun->strict);
        type->constructor.activation = ejsClone(ejs, fun->activation, 0);
        type->constructor.boundThis = 0;
        type->constructor.boundArgs = 0;
        type->constructor.isConstructor = 1;
        type->constructor.block.pot.isBlock = 1;
        type->constructor.block.pot.isFunction = 1;
        type->hasConstructor = 1;
        type->constructor.block.scope = fun->block.scope;
        fun->archetype = type;
    }
    ejsClonePotHelpers(ejs, type);
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

    if (slotNum < 0 || slotNum >= ((EjsPot*) ejs->global)->numProp) {
        return 0;
    }
    type = ejsGetProperty(ejs, ejs->global, slotNum);
    if (type == 0 || !ejsIsType(ejs, type)) {
        return 0;
    }
    return type;
}


EjsPot *ejsGetPrototype(Ejs *ejs, EjsAny *obj)
{
    return ((EjsObj*) obj)->type->prototype;
}


EjsType *ejsGetTypeByName(Ejs *ejs, EjsName qname)
{
    return ejsGetPropertyByName(ejs, ejs->global, qname);
}


static int inheritProperties(Ejs *ejs, EjsType *type, EjsPot *obj, int destOffset, EjsPot *baseBlock, int srcOffset, 
        int count, bool resetScope)
{
    EjsFunction     *fun;
    int             i;

    mprAssert(obj);
    mprAssert(baseBlock);
    mprAssert(count > 0);
    mprAssert(destOffset < obj->numProp);
    mprAssert((destOffset + count) <= obj->numProp);
    mprAssert(srcOffset < baseBlock->numProp);
    mprAssert((srcOffset + count) <= baseBlock->numProp);

    ejsCopySlots(ejs, obj, &obj->properties->slots[destOffset], &baseBlock->properties->slots[srcOffset], count);
    
    if (resetScope) {
        for (i = destOffset; i < (destOffset + count); i++) {
            fun = ejsGetProperty(ejs, obj, i);
            if (ejsIsFunction(ejs, fun)) {
                fun = ejsCloneFunction(ejs, fun, 0);
                ejsSetProperty(ejs, obj, i, fun);
                fun->boundThis = 0;
                fun->boundArgs = 0;
                fun->block.scope = (EjsBlock*) type;
            }
        }
    }
    ejsMakeHash(ejs, obj);
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
        if (type->implements || baseType->prototype->numProp > 0) {
            fixupPrototypeProperties(ejs, type, baseType, makeRoom);
        }
        if (baseType->isPot && baseType != ejs->objectType) {
            type->isPot = 1;
        }
   }
    fixInstanceSize(ejs, type);
    return 0;
}


#if UNUSED && MOB
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
    mprAssert(count == 0);
    //  MOB -- currently not being used

    if (count > 0) { 
        /*  Append properties to the end of the type so as to not mess up the first slot which may be an initializer */
        destOffset = ejsGetPropertyCount(ejs, (EjsObj*) type);
        srcOffset = 0;
        if (ejsGrowPot(ejs, (EjsObj*) type, type->constructor.block.obj.numProp + count) < 0) {
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
        if (ejsGrowPot(ejs, (EjsObj*) type->prototype, type->prototype->numProp + protoCount) < 0) {
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
#endif


static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom)
{
    EjsType         *iface;
    EjsBlock        *bp;
    EjsNamespace    *nsp;
    int             next, offset, itotal, icount, nextNsp;

    mprAssert(type);
    mprAssert(type->implements);

    itotal = 0;
    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (!iface->isInterface) {
            itotal += iface->constructor.block.pot.numProp;
            type->hasInstanceVars |= iface->hasInstanceVars;
        }
    }
    if (makeRoom) {
        offset = type->constructor.block.pot.numProp;
        if (itotal > 0 && ejsGrowPot(ejs, (EjsPot*) type, offset + itotal) < 0) {
            return EJS_ERR;
        }
    } else {
        offset = type->constructor.block.pot.numProp - itotal;
    }
    for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
        if (!iface->isInterface) {
            icount = iface->constructor.block.pot.numProp;
            if (inheritProperties(ejs, type, (EjsPot*) type, offset, (EjsPot*) iface, 0, icount, 1) < 0) {
                return EJS_ERR;
            }
            offset += icount;
            for (nextNsp = 0; 
                    (nsp = (EjsNamespace*) mprGetNextItem(&iface->constructor.block.namespaces, &nextNsp)) != 0;) {
                ejsAddNamespaceToBlock(ejs, (EjsBlock*) type, nsp);
            }
            for (bp = iface->constructor.block.scope; bp->scope; bp = bp->scope) {
                for (nextNsp = 0; (nsp = (EjsNamespace*) mprGetNextItem(&bp->namespaces, &nextNsp)) != 0;) {
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
    EjsPot      *basePrototype;
    int         count, offset, next;

    mprAssert(type != baseType);
    mprAssert(type->prototype);
    mprAssert(baseType->prototype);
    
    basePrototype = baseType->prototype;

    if (makeRoom) {
        count = 0;
        mprAssert(basePrototype);
        /* Must inherit if the type has instance vars */
        if (basePrototype && baseType->hasInstanceVars) {
            count = basePrototype->numProp;
        }
        for (next = 0; ((iface = mprGetNextItem(type->implements, &next)) != 0); ) {
            if (!iface->isInterface && iface->prototype) {
                count += iface->prototype->numProp - iface->numInherited;
            }
        }
        if (count > 0 && ejsInsertPotProperties(ejs, type->prototype, count, 0) < 0) {
            return EJS_ERR;
        }
    }
    offset = 0;
    if (baseType->hasInstanceVars) {
        mprAssert(type->prototype->numProp >= basePrototype->numProp);
        if (inheritProperties(ejs, type, type->prototype, offset, basePrototype, 0, basePrototype->numProp, 0) < 0) {
            return EJS_ERR;
        }
        type->numInherited = basePrototype->numProp;
        offset += basePrototype->numProp;
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
            count = iface->prototype->numProp - iface->numInherited;
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
int ejsBindMethod(Ejs *ejs, EjsAny *obj, int slotNum, void *nativeProc)
{
    return ejsBindFunction(ejs, obj, slotNum, nativeProc);
}


int ejsBindAccess(Ejs *ejs, void *obj, int slotNum, void *getter, void *setter)
{
    EjsFunction     *fun;

    if (getter) {
        if (ejsBindFunction(ejs, obj, slotNum, getter) < 0) {
            return EJS_ERR;
        }
    }
    if (setter) {
        fun = ejsGetProperty(ejs, obj, slotNum);
        if (fun == 0 || !ejsIsFunction(ejs, fun) || fun->setter == 0 || !ejsIsFunction(ejs, fun->setter)) {
            ejs->hasError = 1;
            mprError("Attempt to bind non-existant setter function for slot %d in \"%s\"", slotNum, ejsGetName(obj));
            return EJS_ERR;
        }
        fun = fun->setter;
        if (fun->body.code) {
            mprError("Setting a native method on a non-native function \"%@\" in \"%s\"", fun->name, ejsGetName(obj));
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
int ejsBindFunction(Ejs *ejs, EjsAny *obj, int slotNum, void *nativeProc)
{
    EjsFunction     *fun;

    if (ejsGetPropertyCount(ejs, obj) < slotNum) {
        ejs->hasError = 1;
        mprError("Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, ejsGetName(obj));
        return EJS_ERR;
    }
    fun = ejsGetProperty(ejs, obj, slotNum);
    if (fun == 0 || !ejsIsFunction(ejs, fun)) {
        mprAssert(fun);
        ejs->hasError = 1;
        mprError("Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, ejsGetName(obj));
        return EJS_ERR;
    }
    if (fun->body.code) {
        mprError("Setting a native method on a non-native function \"%@\" in \"%s\"", fun->name, ejsGetName(obj));
        ejs->hasError = 1;
    }
    mprAssert(fun->body.proc == 0);
    fun->body.proc = nativeProc;
    fun->isNativeProc = 1;
    return 0;
}


void ejsBindConstructor(Ejs *ejs, EjsType *type, void *nativeProc)
{
    mprAssert(type->hasConstructor);
    mprAssert(type->constructor.isConstructor);
    mprAssert(type->constructor.block.pot.isBlock);
    mprAssert(type->constructor.block.pot.isFunction);
    mprAssert(type->constructor.body.proc == 0);

    type->constructor.body.proc = nativeProc;
    type->constructor.isNativeProc = 1;
}


/*
    Define a global public function. Returns a positive slot number, otherwise a negative MPR error.
 */
int ejsDefineGlobalFunction(Ejs *ejs, EjsString *name, EjsProc fn)
{
    EjsFunction *fun;
    EjsName     qname;

    if ((fun = ejsCreateFunction(ejs, name, NULL, -1, 0, 0, 0, ejs->objectType, 0, NULL, NULL, 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    fun->body.proc = fn;
    fun->isNativeProc = 1;
    qname.space = ejsCreateStringFromAsc(ejs, EJS_PUBLIC_NAMESPACE);
    qname.name = name;
    return ejsSetPropertyByName(ejs, ejs->global, qname, fun);
}


/*
    Return true if target is an instance of type or a sub class of it.
 */
bool ejsIsA(Ejs *ejs, EjsAny *target, EjsType *type)
{
    mprAssert(type);

    if (!ejsIsType(ejs, type) || target == 0) {
        return 0;
    }
    return ejsIsTypeSubType(ejs, TYPE(target), type);
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
    
    if (!ejsIsType(ejs, target) || !ejsIsType(ejs, type)) {
        return 0;
    }
    /*
        See if target is a subtype of type
     */
    for (tp = target; tp; tp = tp->baseType) {
        /*
            Test ID also to allow worker interpreters to match where the IDs are equal
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
    This call is currently only used to update the type namespace after resolving a run-time namespace.
 */
void ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname)
{
    type->qname = qname;
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
#if UNUSED
    nsp->flags |= EJS_NSP_PROTECTED;
#endif
    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PRIVATE_NAMESPACE);
#if UNUSED
    nsp->flags |= EJS_NSP_PRIVATE;
#endif
}


/*********************************** Factory **********************************/

static void manageType(EjsType *type, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManageFunction(&type->constructor, flags);
        mprMark(type->qname.name);
        mprMark(type->qname.space);
        mprMark(type->prototype);
        mprMark(type->baseType);
        mprMarkList(type->implements);
    }
}


void ejsCreateTypeType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->typeType;
    type->manager = (MprManager) manageType;

    ejsCloneBlockHelpers(ejs, type);

    type->helpers.clone        = (EjsCloneHelper) cloneTypeVar;
    type->helpers.create       = (EjsCreateHelper) createTypeVar;
    type->helpers.setProperty  = (EjsSetPropertyHelper) setTypeProperty;

    /*
        WARNING: read closely. This can be confusing. Fixup the helpers for the object type. We need to find
        helpers via objectType->type->helpers. So we set it to the Type type. We keep objectType->baseType == 0
        because Object has no base type. Similarly for the Type type.
     */
    TYPE(&ejs->objectType->constructor) = ejs->typeType;
    TYPE(&ejs->blockType->constructor) = ejs->typeType;
    TYPE(&ejs->stringType->constructor) = ejs->typeType;
    TYPE(&ejs->typeType->constructor) = ejs->objectType;
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
    vim: sw=8 ts=8 expandtab

    @end
 */
