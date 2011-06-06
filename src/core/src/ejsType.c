/**
    ejsType.c - Type class

    The type class is the base class for all types (classes) in the system.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/***************************** Forward Declarations ***************************/

static EjsType *createBootType(Ejs *ejs, EjsType *baseType, int slotNum, int size, void *manager, int64 attributes);
static EjsType *createTypeVar(Ejs *ejs, EjsType *typeType, int numProp);
static void finishBootType(Ejs *ejs, int slotNum, EjsName qname, EjsType *type);
static void fixInstanceSize(Ejs *ejs, EjsType *type);
static int fixupPrototypeProperties(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
static int fixupTypeImplements(Ejs *ejs, EjsType *type, int makeRoom);
static int inheritProperties(Ejs *ejs, EjsType *type, EjsPot *obj, int destOffset, EjsPot *baseBlock, int srcOffset, 
    int count, bool resetScope);
static void manageDefault(EjsObj *ev, int flags);
static void manageType(EjsType *type, int flags);
static int64 setDefaultAttributes(EjsType *type, int size, int64 attributes);
static void zeroSlots(Ejs *ejs, EjsPot *obj, int count, EjsAny *null);

/******************************************************************************/
/*
    Handcraft the Object, Type, Null, String, and Block types.
 */
int ejsCreateBootstrapTypes(Ejs *ejs)
{
    EjsObj      *null;
    EjsPot      *immutable;
    EjsType     *blockType, *nullType, *objectType, *stringType, *typeType;

    mprAssert(ejs);

    ejsCreateObjHelpers(ejs);
    ejsCreatePotHelpers(ejs);
    ejsCreateBlockHelpers(ejs);

    /*
        Partially create the core types. Bit of a chicken-an-egg problem. Finish creating types below after 
        the "" and null values have been created.
     */
    typeType   = createBootType(ejs, 0, S_Type, sizeof(EjsType), manageType, 
        EJS_TYPE_BLOCK | EJS_TYPE_DYNAMIC_INSTANCES);
    objectType = createBootType(ejs, typeType, S_Object, sizeof(EjsPot), ejsManagePot, 
        EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES);
    blockType  = createBootType(ejs, typeType, S_Block, sizeof(EjsBlock), ejsManageBlock, 
        EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES);
    stringType = createBootType(ejs, typeType, S_String, sizeof(EjsString), ejsManageString, 
        EJS_TYPE_OBJ | EJS_TYPE_IMMUTABLE_INSTANCES);
    nullType   = createBootType(ejs, typeType, S_Null, sizeof(EjsNull), manageDefault, EJS_TYPE_OBJ);
    SET_TYPE(typeType, objectType);

    /*
        Create the immutable set and then manually zero after creating "null"
     */
    ejs->service->immutable = immutable = ejsCreatePot(ejs, objectType, 0);
    ejsGrowPot(ejs, immutable, max(EJS_NUM_GLOBAL, ES_global_NUM_CLASS_PROP));
    mprSetName(immutable, "immutable");
    ESV(null) = null = ejsCreateObj(ejs, nullType, 0);
    mprSetName(ESV(null), "null value");
    zeroSlots(ejs, immutable, immutable->numProp, null);

    /*
        Preset some essential types and values so we can finish up the types
     */
    ESV(Object) = objectType;
    ESV(String) = stringType;
    ESV(null) = null;
    ESV(empty) = ejsCreateStringFromAsc(ejs, "");

    finishBootType(ejs, ES_String, N("ejs", "String"), stringType);
    finishBootType(ejs, ES_Object, N("ejs", "Object"), objectType);
    finishBootType(ejs, ES_Block, N("ejs", "Block"), blockType);
    finishBootType(ejs, ES_Null, N("ejs", "Null"), nullType);
    finishBootType(ejs, ES_Type, N("ejs", "Type"), typeType);

    ejsInitStringType(ejs, stringType);
    ejsInitTypeType(ejs, typeType);
    ejsInitBlockType(ejs, blockType);
    ejsInitNullType(ejs, nullType);
    
    if (ejs->empty) {
        ejs->global = ejsCreateBlock(ejs, 0);
    } else {
        ejs->global = ejsCreateBlock(ejs, max(ES_global_NUM_CLASS_PROP, EJS_NUM_GLOBAL));
        ((EjsPot*) ejs->global)->numProp = ES_global_NUM_CLASS_PROP;
    }
    mprSetName(ejs->global, "global");
    return 0;
}


static EjsType *createBootType(Ejs *ejs, EjsType *baseType, int slotNum, int size, void *manager, int64 attributes)
{
    EjsType     *type;

    mprAssert(0 <= slotNum && slotNum < EJS_MAX_SPECIAL);

    if ((type = createTypeVar(ejs, NULL, 0)) == NULL) {
        return 0;
    }
    SET_TYPE(type, baseType);
    type->sid = slotNum;
    attributes = setDefaultAttributes(type, size, attributes);
    attributes = ejsSetTypeAttributes(type, size, manager, attributes);
    ejsSetTypeHelpers(type, attributes);
    return type;
}


static void finishBootType(Ejs *ejs, int slotNum, EjsName qname, EjsType *type)
{
    type->qname = qname;
    mprSetName(type, qname.name->value);
    ejsAddImmutable(ejs, slotNum, type->qname, type);

    if ((type->prototype = ejsCreatePot(ejs, ESV(Object), 0)) == 0) {
        return;
    }
    mprSetName(type->prototype, qname.name->value);
    type->prototype->isPrototype = 1;
}


/*
    Create a new type and initialize. BaseType is the super class for instances of the type being created. The
    returned EjsType will be an instance of EjsType. numTypeProp and numInstanceProp should be set to the number
    of non-inherited properties.
 */
EjsType *ejsCreateType(Ejs *ejs, EjsName qname, EjsModule *up, EjsType *baseType, EjsPot *prototype, 
        int sid, int numTypeProp, int numInstanceProp, int size, void *manager, int64 attributes)
{
    EjsType     *type;
    
    mprAssert(ejs);
    mprAssert(size >= 0);
    
    if ((type = createTypeVar(ejs, ESV(Type), numTypeProp)) == 0) {
        return 0;
    }
    attributes = setDefaultAttributes(type, size, attributes);
    attributes = ejsSetTypeAttributes(type, size, manager, attributes);
    ejsSetTypeHelpers(type, attributes);
    type->sid = sid;
    type->qname = qname;
    type->constructor.name = qname.name;
    type->module = up;
    type->baseType = baseType;

    //  UNICODE
    mprSetName(type, qname.name->value);

    if (prototype) {
        type->prototype = prototype;
    } else {
        if ((type->prototype = ejsCreatePot(ejs, ESV(Object), numInstanceProp)) == 0) {
            return 0;
        }
        //  UNICODE
        mprSetName(type->prototype, qname.name->value);
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
EjsType *ejsCreateCoreType(Ejs *ejs, EjsName qname, int size, int sid, int numTypeProp, void *manager, int64 attributes)
{
    EjsType     *type;

#if BLD_DEBUG
    if (attributes & EJS_TYPE_POT) {
        if (size > sizeof(EjsPot)) {
            mprAssert(attributes & EJS_TYPE_DYNAMIC_INSTANCES);
            attributes |= EJS_TYPE_DYNAMIC_INSTANCES;
        }
    }
#endif
    if ((type = ejsCreateType(ejs, qname, NULL, NULL, NULL, sid, numTypeProp, 0, size, manager, attributes)) == 0) {
        ejs->hasError = 1;
        return 0;
    }
    if (!type->mutable && type->sid >= 0) {
        ejsAddImmutable(ejs, type->sid, type->qname, type);
    }
    return type;
}


EjsType *ejsFinalizeCoreType(Ejs *ejs, EjsName qname)
{
    EjsType     *type;

    if ((type = ejsGetTypeByName(ejs, qname)) == 0) {
        mprError("Can't find %N type", qname);
        return 0;
    }
    if (type->configured) {
        return 0;
    }
    type->configured = 1;
    return type;
}


EjsType *ejsFinalizeScriptType(Ejs *ejs, EjsName qname, int size, void *manager, int64 attributes)
{
    EjsType     *type;

    if ((type = ejsGetTypeByName(ejs, qname)) == 0) {
        mprError("Can't find %N type", qname);
        return 0;
    }
    if (type->configured) {
        return 0;
    }
#if UNUSED
    if (attributes & EJS_TYPE_POT) {
        if (size > sizeof(EjsPot)) {
            mprAssert(attributes & EJS_TYPE_DYNAMIC_INSTANCES);
            attributes |= EJS_TYPE_DYNAMIC_INSTANCES;
        }
    }
#endif
    attributes = setDefaultAttributes(type, size, attributes);
    attributes = ejsSetTypeAttributes(type, size, manager, attributes);
    ejsSetTypeHelpers(type, attributes);

    if (!type->mutable && type->sid >= 0) {
        ejsAddImmutable(ejs, type->sid, type->qname, type);
    }
    type->manager = manager ? (MprManager) manager : (MprManager) manageDefault;

    type->configured = 1;
    return type;
}


EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, EjsModule *up, EjsType *baseType, int numTypeProp, int numInstanceProp, 
    int64 attributes)
{
    type->module = up;

    /* Don't set default attributes */
    attributes &= ~(EJS_TYPE_BLOCK | EJS_TYPE_OBJ | EJS_TYPE_POT);
    ejsSetTypeAttributes(type, type->instanceSize, type->manager, attributes);
    /* Don't modify helpers */

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

    if (prototype == 0 && fun) {
        prototype = ejsGetPropertyByName(ejs, fun, N(NULL, "prototype"));
    }
    baseType = prototype ? TYPE(prototype): ESV(Object);
    name = (fun && fun->name) ? fun->name : ejsCreateStringFromAsc(ejs, "-type-from-function-");
    qname.space = ejsCreateStringFromAsc(ejs, EJS_PROTOTYPE_NAMESPACE);
    qname.name = name;
    type = ejsCreateType(ejs, qname, NULL, baseType, prototype, -1, 0, 0, EST(Object)->instanceSize, 0, 
        EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES);
    if (type == 0) {
        return 0;
    }
    if (fun) {
        code = fun->body.code;
        /*  using ESV(Object) as the return type because the Yahoo module pattern returns {} in the constructor */
        ejsInitFunction(ejs, (EjsFunction*) type, type->qname.name, code->byteCode, code->codeLen, 
            fun->numArgs, fun->numDefault, code->numHandlers, ESV(Object), EJS_TRAIT_HIDDEN | EJS_TRAIT_FIXED, 
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
    ejsApplyPotHelpers(ejs->service, type);
    return type;
}


/*
    OPT - should be able to just read in the attributes without having to stuff some in var and some in type.
    Should eliminate all the specific fields and just use BIT MASKS.
    NOTE: EJS_TYPE_IMMUTABLE_INSTANCES overrides EJS_TYPE_MUTABLE_INSTANCES.
 */
static int64 setDefaultAttributes(EjsType *type, int size, int64 attributes)
{
    if ((attributes & (EJS_TYPE_POT | EJS_TYPE_BLOCK | EJS_TYPE_OBJ)) == 0) {
        attributes |= EJS_TYPE_POT;
    }
    if (attributes & EJS_TYPE_POT) {
        if (size && size != sizeof(EjsPot)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
        
    } else if (attributes & EJS_TYPE_BLOCK) {
        if (size && size != sizeof(EjsBlock)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
    } else if (attributes & EJS_TYPE_OBJ) {
        if (size && size != sizeof(EjsObj)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
    }
    return attributes;
}


int64 ejsSetTypeAttributes(EjsType *type, int size, MprManager manager, int64 attributes)
{
    if (attributes & EJS_TYPE_POT) {
        type->isPot = 1;
        if (manager == 0) {
            manager = ejsManagePot;
        }
        if (size == 0) {
            size = sizeof(EjsPot);
        }
        if (size != sizeof(EjsBlock)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
        
    } else if (attributes & EJS_TYPE_BLOCK) {
        type->isPot = 1;
        if (manager == 0) {
            manager = (MprManager) ejsManageBlock;
        }
        if (size == 0) {
            size = sizeof(EjsBlock);
        }
        if (size != sizeof(EjsBlock)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
    } else if (attributes & EJS_TYPE_OBJ) {
        type->isPot = 0;
        if (manager == 0) {
            manager = (MprManager) manageDefault;
        }
        if (size == 0) {
            size = sizeof(EjsObj);
        }
        if (size != sizeof(EjsObj)) {
            attributes |= EJS_TYPE_MUTABLE_INSTANCES;
        }
    }
    if (attributes & EJS_TYPE_CALLS_SUPER) {
        type->callsSuper = 1;
    }
    if (attributes & EJS_TYPE_DYNAMIC_INSTANCES) {
        type->dynamicInstances = 1;
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
    if (attributes & EJS_TYPE_INTERFACE) {
        type->isInterface = 1;
    }
    if (attributes & EJS_TYPE_HAS_TYPE_INITIALIZER) {
        type->hasInitializer = 1;
    }
    if (attributes & EJS_TYPE_MUTABLE) {
        mprAssert(type->mutable == 0);
        type->mutable = 1;
    }
    /* IMMUTABLE takes precedence */
    if (attributes & EJS_TYPE_IMMUTABLE_INSTANCES) {
        type->mutableInstances = 0;
    } else if (attributes & EJS_TYPE_MUTABLE_INSTANCES) {
        type->mutableInstances = 1;
    }
    if (attributes & EJS_TYPE_NUMERIC_INDICIES) {
        type->numericIndicies = 1;
    } 
    if (attributes & EJS_TYPE_VIRTUAL_SLOTS) {
        type->virtualSlots = 1;
    }
    type->manager = manager;
    type->instanceSize = size;
    return attributes;
}


void ejsSetTypeHelpers(EjsType *type, int64 attributes)
{
    if (attributes & EJS_TYPE_BLOCK) {
        ejsApplyBlockHelpers(MPR->ejsService, type);
        type->isPot = 1;

    } else if (attributes & EJS_TYPE_POT) {
        ejsApplyPotHelpers(MPR->ejsService, type);
        type->isPot = 1;

    } else if (attributes & EJS_TYPE_OBJ) {
        ejsApplyObjHelpers(MPR->ejsService, type);
        type->isPot = 0;
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
    return TYPE(obj)->prototype;
}


EjsType *ejsGetTypeByName(Ejs *ejs, EjsName qname)
{
    return ejsGetPropertyByName(ejs, ejs->global, qname);
}


static int inheritProperties(Ejs *ejs, EjsType *type, EjsPot *obj, int destOffset, EjsPot *baseBlock, int srcOffset, 
        int count, bool resetScope)
{
    EjsFunction     *fun;
    EjsObj          *vp;
    int             i;

    mprAssert(obj);
    mprAssert(baseBlock);
    mprAssert(count > 0);
    mprAssert(destOffset < obj->numProp);
    mprAssert((destOffset + count) <= obj->numProp);
    mprAssert(srcOffset < baseBlock->numProp);
    mprAssert((srcOffset + count) <= baseBlock->numProp);

    ejsCopySlots(ejs, obj, destOffset, baseBlock, srcOffset, count);
    
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
    for (i = destOffset; i < (destOffset + count); i++) {
        if ((vp = ejsGetProperty(ejs, obj, i)) != 0 && !ejsIs(ejs, vp, Null) && !ejsIsFunction(ejs, vp)) {
            if (ejsIsType(ejs, vp)) {
                ejsSetProperty(ejs, obj, i, ESV(null));
            }
        }
    }
    ejsMakeHash(ejs, obj);
    return 0;
}


static void fixInstanceSize(Ejs *ejs, EjsType *type)
{
    EjsType     *tp;

    for (tp = type->baseType; tp && tp != ESV(Object); tp = tp->baseType) {
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
        //  TODO-- should be able to remove the || baseType->hasBaseConstructors
        if (baseType->hasConstructor || baseType->hasBaseConstructors) {
            type->hasBaseConstructors = 1;
        }
        //  TODO -- when compiling baseType is always != ESV(Object)
        //  TODO - should not explicity reference objecttype
        if (baseType != ESV(Object) && baseType->dynamicInstances) {
            type->dynamicInstances = 1;
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
        if (baseType->isPot && baseType != ESV(Object)) {
            type->isPot = 1;
        }
   }
    fixInstanceSize(ejs, type);
    return 0;
}


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
            for (bp = iface->constructor.block.scope; bp; bp = bp->scope) {
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
            mprError("Attempt to bind non-existant setter function for slot %d in \"%s\"", slotNum, mprGetName(obj));
            return EJS_ERR;
        }
        fun = fun->setter;
        if (fun->body.code) {
            mprError("Setting a native method on a non-native function \"%@\" in \"%s\"", fun->name, mprGetName(obj));
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

    if (ejsGetLength(ejs, obj) < slotNum) {
        ejs->hasError = 1;
        mprError("Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, mprGetName(obj));
        return EJS_ERR;
    }
    fun = ejsGetProperty(ejs, obj, slotNum);
    if (fun == 0 || !ejsIsFunction(ejs, fun)) {
        mprAssert(fun);
        ejs->hasError = 1;
        mprError("Attempt to bind non-existant function for slot %d in \"%s\"", slotNum, mprGetName(obj));
        return EJS_ERR;
    }
    if (fun->body.code) {
        mprError("Setting a native method on a non-native function \"%@\" in \"%s\"", fun->name, mprGetName(obj));
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

    if ((fun = ejsCreateFunction(ejs, name, NULL, -1, 0, 0, 0, ESV(Object), 0, NULL, NULL, 0)) == 0) {
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
            All strings are interned, so this will test equality of types even across interpreters.
            This permits using code from another interpreter.
         */
        if (tp->qname.name == type->qname.name && tp->qname.space == type->qname.space) {
            return 1;
        }
    }
    /*
        See if target implements type
     */
    if (target->implements) {
        for (next = 0; (iface = mprGetNextItem(target->implements, &next)) != 0; ) {
            if (iface->qname.name == type->qname.name && iface->qname.space == type->qname.space) {
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
    nsp = ejsDefineReservedNamespace(ejs, (EjsBlock*) type, &type->qname, EJS_PRIVATE_NAMESPACE);
}


static void zeroSlots(Ejs *ejs, EjsPot *obj, int count, EjsAny *null)
{
    EjsSlot     *slots, *sp;

    mprAssert(obj);
    mprAssert(count >= 0);

    slots = obj->properties->slots;
    for (sp = &slots[count - 1]; sp >= slots; sp--) {
        sp->value.ref = null;
        sp->hashChain = -1;
        sp->trait.type = 0;
        sp->trait.attributes = 0;
    }
}

/************************************ Helpers *********************************/
/*
    Copy a type

    function copy(type: Object): Object
 */
static EjsType *cloneTypeVar(Ejs *ejs, EjsType *src, bool deep)
{
    EjsType     *dest;

    if (! ejsIsType(ejs, src)) {
        ejsThrowTypeError(ejs, "Expecting a Type object");
        return 0;
    }
    dest = (EST(Function)->helpers.clone)(ejs, src, deep);
    if (dest == 0) {
        return dest;
    }
    //  TODO OPT
    dest->baseType = src->baseType;
    dest->callsSuper = src->callsSuper;
    dest->dynamicInstances = src->dynamicInstances;
    dest->final = src->final;
    dest->hasBaseConstructors = src->hasBaseConstructors;
    dest->hasBaseInitializers = src->hasBaseInitializers;
    dest->hasConstructor = src->hasConstructor;
    dest->hasInitializer = src->hasInitializer;
    dest->hasInstanceVars = src->hasInstanceVars;
    dest->hasMeta = src->hasMeta;
    dest->hasScriptFunctions = src->hasScriptFunctions;
    dest->helpers = src->helpers;
    dest->implements = src->implements;
    dest->initialized = src->initialized;
    dest->instanceSize = src->instanceSize;
    dest->isInterface = src->isInterface;
    dest->isPot = src->isPot;
    dest->manager = src->manager;
    dest->mutable = src->mutable;
    dest->mutableInstances = src->mutableInstances;
    dest->mutex = src->mutex;
    dest->module = src->module;
    dest->numericIndicies = src->numericIndicies;
    dest->numInherited = src->numInherited;
    dest->prototype = src->prototype;
    dest->qname = src->qname;
    dest->sid = src->sid;
    dest->typeData = src->typeData;
    dest->virtualSlots = src->virtualSlots;
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
    ssize       typeSize;
    int         sizeHash, dynamic;

    mprAssert(ejs);
    
    /*
        If the compiler is building itself (empty mode), then the types themselves must be dynamic. Otherwise, the type
        is fixed and will contain the names hash and traits in one memory block. 
        NOTE: don't confuse this with dynamic objects.
     */
    sizeHash = 0;
    if (numProp < 0 || ejs->empty) {
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
    obj = (EjsPot*) type;
    SET_TYPE(obj, typeType);
    SET_DYNAMIC(obj, dynamic);
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
    return (EST(Block)->helpers.setProperty)(ejs, type, slotNum, value);
}


/*********************************** Factory **********************************/
/*
    Default manager for instances
 */
static void manageDefault(EjsObj *ev, int flags)
{
#if BLD_DEBUG
    if (flags & MPR_MANAGE_MARK) {
        mprAssert(!TYPE(ev)->isPot);
    }
#endif
}


static void manageType(EjsType *type, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManageFunction(&type->constructor, flags);
        mprMark(type->qname.name);
        mprMark(type->qname.space);
        mprMark(type->prototype);
        mprMark(type->baseType);
        if (type->mutex) {
            mprNop(NULL);
            mprMark(type->mutex);
        }
        mprMark(type->implements);
        mprMark(type->module);
        mprMark(type->typeData);
    }
}


void ejsInitTypeType(Ejs *ejs, EjsType *type)
{
    type->helpers.clone        = (EjsCloneHelper) cloneTypeVar;
    type->helpers.create       = (EjsCreateHelper) createTypeVar;
    type->helpers.setProperty  = (EjsSetPropertyHelper) setTypeProperty;
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
