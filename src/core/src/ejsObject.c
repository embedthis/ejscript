/**
    ejsObject.c - Object class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Defines **********************************/

#define CMP_QNAME(a,b) cmpQname(a, b)
#define CMP_NAME(a,b) cmpName(a, b)

/****************************** Forward Declarations **************************/

static int      cmpName(EjsName *a, EjsName *b);
static int      cmpQname(EjsName *a, EjsName *b);
static EjsName  getObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum);
static int      growSlots(Ejs *ejs, EjsObj *obj, int size);
static int      hashProperty(EjsObj *obj, int slotNum, EjsName *qname);
static int      lookupObjectProperty(struct Ejs *ejs, EjsObj *obj, EjsName *qname);
static EjsObj   *obj_defineProperty(Ejs *ejs, EjsObj *type, int argc, EjsObj **argv);
static EjsObj   *obj_toString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv);
static void     removeHashEntry(Ejs *ejs, EjsObj  *obj, EjsName *qname);

/************************************* Code ***********************************/
/*
    Cast the operand to another type
 */
static EjsObj *castObject(Ejs *ejs, EjsObj *obj, EjsType *type)
{
    EjsString       *str;
    EjsFunction     *fun;
    EjsObj          *result;
    EjsLookup       lookup;
    EjsName         qname;
    int             enabled;
    
    mprAssert(ejsIsType(type));

    if (type->hasMeta) {
        ejsName(&qname, EJS_META_NAMESPACE, "cast");
        return ejsRunFunctionByName(ejs, (EjsObj*) type, &qname, (EjsObj*) type, 1, &obj);
    }

    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ejsCreateBoolean(ejs, 1);

    case ES_Number:
        str = ejsToString(ejs, (EjsObj*) obj);
        if (str == 0) {
            ejsThrowMemoryError(ejs);
            return 0;
        }
        return ejsParse(ejs, ejsGetString(ejs, str), ES_Number);

    case ES_String:
        if (!obj->isType && !obj->isPrototype) {
            if (ejsLookupVar(ejs, obj, ejsName(&qname, "", "toString"), &lookup) >= 0 && 
                    lookup.obj != ejs->objectType->prototype) {
                fun = ejsGetProperty(ejs, lookup.obj, lookup.slotNum);
                if (fun && ejsIsFunction(fun) && fun->body.proc != obj_toString) {
                    /* Disable GC because toString is used pervasively in native code */
                    enabled = ejs->gc.enabled;
                    ejs->gc.enabled = 0;
                    result = (EjsObj*) ejsRunFunction(ejs, fun, obj, 0, NULL);
                    ejs->gc.enabled = enabled;
                    return result;
                }
            }
        }
        if (obj == ejs->global) {
            return (EjsObj*) ejsCreateString(ejs, "[object global]");
        } else {
            if (obj->type->helpers.cast && obj->type->helpers.cast != castObject) {
                return (obj->type->helpers.cast)(ejs, obj, type);
            }
            return (EjsObj*) ejsCreateStringAndFree(ejs, mprStrcat(ejs, -1, "[object ", obj->type->qname.name, "]", NULL));
        }

    default:
        if (ejsIsA(ejs, (EjsObj*) obj, type)) {
            return (EjsObj*) obj;
        }
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


/*
    Cast the operands depending on the operation code
 */
EjsObj *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {

    /*
        Binary operators
     */
    case EJS_OP_ADD:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejs->zeroValue, opcode, rhs);

    case EJS_OP_COMPARE_EQ:  case EJS_OP_COMPARE_NE:
        if (ejsIsNull(rhs) || ejsIsUndefined(rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        } else if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    /* Unary operators */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


EjsObj *ejsObjectOperator(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || lhs->type != rhs->type) {
        if ((result = ejsCoerceOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }
    /* Types now match */
    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
        return (EjsObj*) ejsCreateBoolean(ejs, (lhs == rhs));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
        return (EjsObj*) ejsCreateBoolean(ejs, !(lhs == rhs));

    /* Unary operators */

    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return (EjsObj*) ejs->oneValue;

    /* Binary operators */

    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL:
    case EJS_OP_REM: case EJS_OP_OR: case EJS_OP_SHL: case EJS_OP_SHR:
    case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, (EjsObj*) ejsToNumber(ejs, rhs));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->type->qname.name);
        return 0;
    }
    mprAssert(0);
}


/*
    Create an object which is an instance of a given type. NOTE: only initialize the Object base class. It is up to the 
    caller to complete the initialization for all other base classes by calling the appropriate constructors. The numSlots 
    arg is the number of property slots to pre-allocate. It is typically zero and slots are allocated on-demand. If the 
    type creates dynamic instances, then the property slots are allocated separately and can grow. 
 */
void *ejsCreateObject(Ejs *ejs, EjsType *type, int numSlots)
{
    EjsObj      *obj, *prototype;

    mprAssert(type);
    
    prototype = type->prototype;
    if (type->hasInstanceVars) {
        numSlots = max(numSlots, prototype->numSlots);
    }
    if (type->dynamicInstance) {
        if ((obj = ejsAllocVar(ejs, type, 0)) == 0) {
            return 0;
        }
        if (numSlots > 0) {
            growSlots(ejs, obj, numSlots);
        }
        obj->dynamic = 1;
    } else {
        if ((obj = ejsAllocVar(ejs, type, numSlots * sizeof(EjsSlot))) == 0) {
            return 0;
        }
        if (numSlots > 0) {
            obj->slots = (EjsSlot*) &(((char*) obj)[type->instanceSize]);
            obj->sizeSlots = numSlots;
        }
    }
    obj->numSlots = numSlots;
    obj->type = type;
    ejsSetDebugName(obj, type->qname.name);

    if (obj->sizeSlots > 0) {
        if (type->hasInstanceVars) {
            if (prototype->numSlots > 0) {
                ejsCopySlots(ejs, obj, obj->slots, prototype->slots, prototype->numSlots, 0);
            }
            ejsZeroSlots(ejs, &obj->slots[prototype->numSlots], obj->sizeSlots - prototype->numSlots);
            if (numSlots > EJS_HASH_MIN_PROP) {
                ejsMakeObjHash(obj);
            }
        } else {
            ejsZeroSlots(ejs, obj->slots, obj->sizeSlots);
        }
    }
    return obj;
}


EjsObj *ejsCreateSimpleObject(Ejs *ejs)
{
    return ejsCreateObject(ejs, ejs->objectType, 0);
}


void *ejsCloneObject(Ejs *ejs, void *vp, bool deep)
{
    EjsObj          *dest, *src;
    EjsSlot         *dp, *sp;
    int             numSlots, i;

    src = (EjsObj*) vp;
    numSlots = src->numSlots;
    dest = ejsCreateObject(ejs, src->type, numSlots);
    if (dest == 0) {
        return 0;
    }
    dest->numSlots = numSlots;
    //  MOB - OPT make a flags word
    dest->builtin = src->builtin;
    dest->dynamic = src->dynamic;
    dest->isFunction = src->isFunction;
    dest->isPrototype = src->isPrototype;
    dest->isType = src->isType;
    dest->permanent = src->permanent;
    dest->shortScope = src->shortScope;

    //  MOB - OPT name copying
#if UNUSED
    dp = dest->slots;
    sp = src->slots;
    for (i = 0; i < numSlots; i++, sp++, dp++) {
        *dp = *sp;
        //  MOB -- all native types must provide clone or set immutable
        if (deep && !sp->value.ref->type->immutable) {
            dp->value.ref = ejsClone(ejs, sp->value.ref, deep);
        }
    }
#else
    dp = dest->slots;
    sp = src->slots;
    for (i = 0; i < numSlots; i++, sp++, dp++) {
        *dp = *sp;
        dp->trait = sp->trait;
        //  MOB - OPT name copying
        dp->qname.space = mprStrdup(dest, sp->qname.space);
        dp->qname.name = mprStrdup(dest, sp->qname.name);
        dp->hashChain = -1;
        if (deep && !sp->value.ref->type->immutable) {
            dp->value.ref = ejsClone(ejs, sp->value.ref, deep);
        }
    }
    if (dest->numSlots > EJS_HASH_MIN_PROP) {
        ejsMakeObjHash(dest);
    }
#endif
    ejsSetDebugName(dest, mprGetName(src));
    return dest;
}


static EjsObj *prepareAccessors(Ejs *ejs, EjsObj *obj, int slotNum, int64 *attributes, EjsObj *value)
{
    EjsFunction     *fun;
    EjsTrait        *trait;

    fun = ejsGetProperty(ejs, obj, slotNum);

    if (*attributes & EJS_TRAIT_SETTER) {
        if (ejsIsFunction(fun)) {
            /* Existing getter, add a setter */
            fun->setter = (EjsFunction*) value;
            if ((trait = ejsGetTrait(ejs, obj, slotNum)) != 0) {
                *attributes |= trait->attributes;
            }
        } else {
            /* No existing getter, must define a dummy getter - will not be called */
            fun = (EjsFunction*) ejsCloneFunction(ejs, ejs->nopFunction, 0);
            fun->setter = (EjsFunction*) value;
        }
        value = (EjsObj*) fun;

    } else if (*attributes & EJS_TRAIT_GETTER) {
        if (ejsIsFunction(fun) && ejsHasTrait(obj, slotNum, EJS_TRAIT_SETTER)) {
            /* Existing getter and setter - preserve any defined setter, overwrite getter */
            if (fun->setter) {
                ((EjsFunction*) value)->setter = fun->setter;
                *attributes |= EJS_TRAIT_SETTER;
            }
        }
    }
    return value;
}


/*
    Define (or redefine) a property and set its name, type, attributes and property value.
 */
static int defineObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsName *qname, EjsType *propType, int64 attributes, 
    EjsObj *value)
{
    EjsFunction     *fun;
    EjsType         *type;
    int             priorSlot;

    mprAssert(ejs);
    mprAssert(slotNum >= -1);
    mprAssert(qname);

    if (attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER) && !ejsIsFunction(value)) {
        ejsThrowTypeError(ejs, "Property \"%s\" is not a function", qname->name);
        return 0;
    }
    priorSlot = ejsLookupProperty(ejs, obj, qname);
    if (slotNum < 0) {
        if (priorSlot < 0) {
            slotNum = ejsGetPropertyCount(ejs, obj);
        } else {
            slotNum = priorSlot;
        }
    }
    if (priorSlot < 0 && ejsSetPropertyName(ejs, obj, slotNum, qname) < 0) {
        return EJS_ERR;
    }
    if (attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER)) {
        value = prepareAccessors(ejs, obj, slotNum, &attributes, value);
    }
    if (value) {
        if (ejsSetProperty(ejs, obj, slotNum, value ? value: ejs->nullValue) < 0) {
            return EJS_ERR;
        }
    }
    if (ejsSetTraitDetails(ejs, (EjsObj*) obj, slotNum, propType, (int) attributes) < 0) {
        return EJS_ERR;
    }

    //  MOB -- reconsider this code
    if (value && ejsIsFunction(value)) {
        fun = ((EjsFunction*) value);
        if (!ejsIsNativeFunction(fun) && ejsIsType(obj)) {
            ((EjsType*) obj)->hasScriptFunctions = 1;
        }
        if (fun->staticMethod && ejsIsType(obj)) {
            type = (EjsType*) obj;
            if (!type->isInterface) {
                /* For static methods, find the right base class and set thisObj to speed up later invocations */
                fun->boundThis = obj;
            }
        }
    }
    return slotNum;
}


/*
    Delete an instance property. To delete class properties, use the type as the obj. This sets the property to null.
    It does not reclaim the property slot.
 */
static int deleteObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsName     qname;
    EjsSlot     *sp;

    mprAssert(obj);
    mprAssert(obj->type);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        ejsThrowReferenceError(ejs, "Invalid property slot to delete");
        return EJS_ERR;
    }
#if UNUSED
    //  MOB -- this should be in the VM and not here
    if (!obj->dynamic) {
        //  MOB -- probably can remove this and rely on fixed below as per ecma spec
        ejsThrowTypeError(ejs, "Can't delete properties in a non-dynamic object");
        return EJS_ERR;
    } else if (ejsHasTrait(obj, slotNum, EJS_TRAIT_FIXED)) {
        ejsThrowTypeError(ejs, "Property \"%s\" is not deletable", qname.name);
        return EJS_ERR;
    }
#endif
    qname = getObjectPropertyName(ejs, obj, slotNum);
    if (qname.name) {
        removeHashEntry(ejs, obj, &qname);
    }
    sp = &obj->slots[slotNum];
    sp->value.ref = ejs->undefinedValue;
    sp->trait.type = 0;
    sp->trait.attributes = EJS_TRAIT_DELETED | EJS_TRAIT_HIDDEN;
    return 0;
}


static int deleteObjectPropertyByName(Ejs *ejs, EjsObj *obj, EjsName *qname)
{
    int     slotNum;

    slotNum = lookupObjectProperty(ejs, obj, qname);
    if (slotNum < 0) {
        ejsThrowReferenceError(ejs, "Property does not exist");
        return EJS_ERR;
    }
    return deleteObjectProperty(ejs, obj, slotNum);
}


static void destroyObject(Ejs *ejs, EjsObj *vp)
{
    ejsFreeVar(ejs, vp, -1);
}


static EjsObj *getObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (obj->slots == 0 || slotNum < 0 || slotNum >= obj->numSlots) {
        ejsThrowReferenceError(ejs, "Property at slot \"%d\" is not found", slotNum);
        return 0;
    }
    return obj->slots[slotNum].value.ref;
}


static int getObjectPropertyCount(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    return obj->numSlots;
}


static EjsName getObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsName     qname;

    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        qname.name = 0;
        qname.space = 0;
        return qname;
    }
    return obj->slots[slotNum].qname;
}


#if UNUSED
static EjsTrait *getObjectPropertyTrait(Ejs *ejs, EjsObj *obj, int slotNum)
{
    return ejsGetTrait(ejs, obj, slotNum);
}
#endif


/*
    Lookup a property with a namespace qualifier in an object and return the slot if found. Return EJS_ERR if not found.
    If qname.space is NULL, then only return a positive slot if there is only one property of the given name.
    Only the name portion is hashed. The namespace is not included in the hash. This is used to do a one-step lookup 
    for properties regardless of the namespace.
 */
static int lookupObjectProperty(struct Ejs *ejs, EjsObj *obj, EjsName *qname)
{
    EjsSlot     *slots, *sp, *np;
    int         slotNum, index, prior;

    mprAssert(qname);
    mprAssert(qname->name);

    slots = obj->slots;
    if (obj->hash == 0) {
        /* No hash. Just do a linear search */
        if (qname->space) {
            for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
                sp = &slots[slotNum];
                if (CMP_QNAME(&sp->qname, qname)) {
                    return slotNum;
                }
            }
            return -1;
        } else {
            for (slotNum = 0, prior = -1; slotNum < obj->numSlots; slotNum++) {
                sp = &slots[slotNum];
                if (CMP_NAME(&sp->qname, qname)) {
                    if (prior >= 0) {
                        /* Multiple properties with the same name */
                        return -1;
                    }
                    prior = slotNum;
                }
            }
            return prior;
        }

    } else {
        /*
            Find the property in the hash chain if it exists. Note the hash does not include the namespace portion.
            We assume that names rarely clash with different namespaces. We do this so variable lookup and do a one
            hash probe and find matching names. Lookup will then pick the right namespace.
         */
        index = ejsComputeHashCode(obj, qname);
        if (qname->space) {
            mprAssert(obj->hash);
            mprAssert(obj->hash->buckets);
            mprAssert(index < obj->hash->size);
            for (slotNum = obj->hash->buckets[index]; slotNum >= 0; slotNum = slots[slotNum].hashChain) {
                sp = &slots[slotNum];
                if (CMP_QNAME(&sp->qname, qname)) {
                    return slotNum;
                }
            }
        } else {
            for (slotNum = obj->hash->buckets[index]; slotNum >= 0; slotNum = sp->hashChain) {
                sp = &slots[slotNum];
                if (CMP_NAME(&sp->qname, qname)) {
                    /* Now ensure there are no more matching names - must be unique in the "name" only */
                    for (np = sp; np->hashChain >= 0; ) {
                        np = &slots[np->hashChain];
                        if (CMP_NAME(&sp->qname, &np->qname)) {
                            /* Multiple properties with the same name */
                            return -1;
                        }
                    }
                    return slotNum;
                }
            }
        }
    }
    return -1;
}


/*
    Mark the object properties for the garbage collector
 */
void ejsMarkObject(Ejs *ejs, void *vp)
{
    EjsSlot     *sp;
    EjsObj      *obj;
    int         i;

    obj = (EjsObj*) vp;

    if (!obj->type->constructor.block.obj.marked) {
        ejsMark(ejs, (EjsObj*) obj->type);
    }
    sp = obj->slots;
    for (i = 0; i < obj->numSlots; i++, sp++) {
        if (sp->value.ref != ejs->nullValue) {      //  MOB test permanent
            ejsMark(ejs, sp->value.ref);
        }
    }
}


/*
    Validate the supplied slot number. If set to -1, then return the next available property slot number.
    Grow the object if required and update numSlots
 */
int ejsGetSlot(Ejs *ejs, EjsObj *obj, int slotNum)
{
    mprAssert(obj->numSlots <= obj->sizeSlots);

    if (slotNum < 0) {
        //  MOB - should this be here or only in the VM. probably only in the VM.
        //  MOB -- or move this routine to the VM
        if (!obj->dynamic) {
            if (obj == ejs->nullValue) {
                ejsThrowReferenceError(ejs, "Object is null");
            } else if (obj == ejs->undefinedValue) {
                ejsThrowReferenceError(ejs, "Object is undefined");
            } else {
                ejsThrowReferenceError(ejs, "Object is not extendable");
            }
            return EJS_ERR;
        }
        slotNum = obj->numSlots++;
        if (slotNum >= obj->sizeSlots) {
            if (growSlots(ejs, obj, obj->numSlots) < 0) {
                ejsThrowMemoryError(ejs);
                return EJS_ERR;
            }
        }
    } else if (slotNum >= obj->numSlots) {
        if (slotNum >= obj->sizeSlots) {
            if (growSlots(ejs, obj, slotNum + 1) < 0) {
                ejsThrowMemoryError(ejs);
                return EJS_ERR;
            }
        }
        obj->numSlots = slotNum + 1;
    }
    mprAssert(obj->numSlots <= obj->sizeSlots);
    return slotNum;
}


/**
    Set the value of a property.
    @param slot If slot is -1, then allocate the next free slot
    @return Return the property slot if successful. Return < 0 otherwise.
 */
static int setObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsObj *value)
{
    mprAssert(ejs);
    mprAssert(obj);
    
    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    mprAssert(slotNum < obj->numSlots);
    mprAssert(obj->numSlots <= obj->sizeSlots);

    mprAssert(value);
#if UNUSED && KEEP
    /*
        This makes anything stored in a global block permanent
        NOTE: permanent objects are still traversed, they are just never swept. If that changes and permanent objects
        are not marked, then this will be required.
     */
    value->permanent |= obj->permanent;
#endif
    obj->slots[slotNum].value.ref = value;
    return slotNum;
}


/*
    Set the name for a property. Objects maintain a hash lookup for property names. This is hash is created on demand 
    if there are more than N properties. If an object is not dynamic, it will use the types name hash. If dynamic, 
    then the types name hash will be copied when required. 
    
    MOB WARNING: Callers must supply persistent names strings in qname
 */
static int setObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum, EjsName *qname)
{
    mprAssert(obj);
    mprAssert(qname);
    mprAssert(qname->name);
    mprAssert(qname->space);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    mprAssert(slotNum < obj->numSlots);

    /* Remove the old hash entry if the name will change */
    if (obj->slots[slotNum].hashChain >= 0) {
        if (CMP_QNAME(&obj->slots[slotNum].qname, qname)) {
            return slotNum;
        }
        removeHashEntry(ejs, obj, &obj->slots[slotNum].qname);
    }
    /* Set the property name. NOTE: the name must be persistent.  */
    obj->slots[slotNum].qname = *qname;
    
    mprAssert(slotNum < obj->numSlots);
    mprAssert(obj->numSlots <= obj->sizeSlots);
    
    if (obj->hash || obj->numSlots > EJS_HASH_MIN_PROP) {
        if (hashProperty(obj, slotNum, qname) < 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }
    }
#if BLD_DEBUG
    {
        EjsObj  *value;

        value = obj->slots[slotNum].value.ref;
        if (value != ejs->nullValue && *mprGetName(value) == '\0') {
            ejsSetDebugName(value, qname->name);
        }
    }
#endif
    return slotNum;
}


#if UNUSED
/*
    Set the property Trait. Grow traits if required.
 */
static int setObjectPropertyTrait(Ejs *ejs, EjsObj *obj, int slotNum, EjsType *type, int attributes)
{
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    obj->slots[slotNum].trait.type = type;
    obj->slots[slotNum].trait.attributes = attributes;
    return slotNum;
}
#endif


/******************************* Slot Routines ********************************/
/*
    Grow and object and update numSlots and numTraits if required
 */
int ejsGrowObject(Ejs *ejs, EjsObj *obj, int numSlots)
{
    if (numSlots > obj->sizeSlots) {
        if (growSlots(ejs, obj, numSlots) < 0) {
            return EJS_ERR;
        }
    }
    if (numSlots > obj->numSlots) {
        obj->numSlots = numSlots;
    }
    return 0;
}


//  MOB -- inconsistent with growObject which takes numSlots. This takes incr.

/*
    Grow the slots, traits, and names by the specified "incr". The new slots|traits|names are created at the "offset"
    Does not update numSlots or numTraits.
 */
int ejsInsertGrowObject(Ejs *ejs, EjsObj *obj, int incr, int offset)
{
    EjsSlot         *sp;
    int             i, size, mark;

    mprAssert(obj);
    mprAssert(incr >= 0);

    if (incr <= 0) {
        return 0;
    }
    size = obj->numSlots + incr;
    if (obj->sizeSlots < size) {
        if (growSlots(ejs, obj, size) < 0) {
            return EJS_ERR;
        }
    }
    obj->numSlots += incr;
    mprAssert(obj->numSlots <= obj->sizeSlots);
    for (mark = offset + incr, i = obj->numSlots - 1; i >= mark; i--) {
        sp = &obj->slots[i - mark];
        obj->slots[i] = *sp;
    }
    ejsZeroSlots(ejs, &obj->slots[offset], incr);
    if (ejsMakeObjHash(obj) < 0) {
        return EJS_ERR;
    }   
    return 0;
}


/*
    Allocate or grow the slots storage for an object. Does not update numSlots.
 */
static int growSlots(Ejs *ejs, EjsObj *obj, int sizeSlots)
{
    EjsSlot     *slots;
    int         factor;

    mprAssert(obj);
    mprAssert(sizeSlots > 0);
    mprAssert(sizeSlots > obj->sizeSlots);

    if (sizeSlots > obj->sizeSlots) {
        if (obj->sizeSlots > EJS_LOTSA_PROP) {
            factor = max(obj->sizeSlots / 4, EJS_ROUND_PROP);
            sizeSlots = (sizeSlots + factor) / factor * factor;
        }
        sizeSlots = EJS_PROP_ROUNDUP(sizeSlots);

        if (obj->slots == 0) {
            mprAssert(obj->sizeSlots == 0);
            mprAssert(sizeSlots > 0);
            obj->slots = (EjsSlot*) mprAlloc(obj, sizeof(EjsSlot) * sizeSlots);
            if (obj->slots == 0) {
                return EJS_ERR;
            }
            ejsZeroSlots(ejs, obj->slots, sizeSlots);
            obj->separateSlots = 1;
        } else {
            if (obj->separateSlots) {
                mprAssert(obj->sizeSlots > 0);
                obj->slots = (EjsSlot*) mprRealloc(obj, obj->slots, sizeof(EjsSlot) * sizeSlots);
            } else {
                slots = (EjsSlot*) mprAlloc(obj, sizeof(EjsSlot) * sizeSlots);
                memcpy(slots, obj->slots, obj->sizeSlots * sizeof(EjsSlot));
                obj->slots = slots;
                obj->separateSlots = 1;
            }
            if (obj->slots == 0) {
                return EJS_ERR;
            }
            ejsZeroSlots(ejs, &obj->slots[obj->sizeSlots], (sizeSlots - obj->sizeSlots));
        }
        obj->sizeSlots = sizeSlots;
    }
    mprAssert(obj->numSlots <= obj->sizeSlots);
    return 0;
}


/*
    Remove a slot and name. Copy up all other properties. WARNING: this can only be used before property binding and 
    should only be used by the compiler.
 */
static void removeSlot(Ejs *ejs, EjsObj *obj, int slotNum, int compact)
{
    int         i;

    mprAssert(obj);
    mprAssert(slotNum >= 0);
    mprAssert(compact);

    if (compact) {
        for (i = slotNum + 1; i < obj->numSlots; i++) {
            obj->slots[i - 1] = obj->slots[i];
        }
        obj->numSlots--;
        i--;
    } else {
        i = slotNum;
    }
    ejsZeroSlots(ejs, &obj->slots[i], 1);
    ejsMakeObjHash(obj);
}


void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count)
{
    EjsSlot     *sp;

    mprAssert(slots);
    mprAssert(count >= 0);

    if (slots) {
        //  TODO OPT. If hashChans were biased by +1 and NULL was allowed for names, then a simple zero would suffice.
        for (sp = &slots[count - 1]; sp >= slots; sp--) {
            sp->value.ref = ejs->nullValue;
            sp->hashChain = -1;
            sp->qname.name = "";
            sp->qname.space = "";
            sp->trait.type = 0;
            sp->trait.attributes = 0;
        }
    }
}


void ejsCopySlots(Ejs *ejs, EjsObj *obj, EjsSlot *dest, EjsSlot *src, int count, int dup)
{
    while (count-- > 0) {
        *dest = *src;
        dest->hashChain = -1;
        if (dup) {
            //  TOOD
            dest->qname.space = mprStrdup(obj, src->qname.space);
            dest->qname.name = mprStrdup(obj, src->qname.name);
        }
        dest++;
        src++;
    }
}

/*********************************** Traits ***********************************/

void ejsSetTraitType(EjsTrait *trait, EjsType *type)
{
    mprAssert(trait);
    mprAssert(type == 0 || ejsIsType(type));
    trait->type = type;
}


void ejsSetTraitAttributes(EjsTrait *trait, int attributes)
{
    mprAssert(trait);
    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);
    trait->attributes = attributes;
}


EjsTrait *ejsGetTrait(Ejs *ejs, void *vp, int slotNum)
{
    EjsObj      *obj;

    obj = (EjsObj*) vp;
    mprAssert(obj);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        //  MOB -- should this grow and allocate (probably)
        return 0;
    }
    return &obj->slots[slotNum].trait;
}


int ejsSetTraitDetails(Ejs *ejs, void *vp, int slotNum, EjsType *type, int attributes)
{
    EjsObj      *obj;

    obj = (EjsObj*) vp;

    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    obj->slots[slotNum].trait.type = type;
    obj->slots[slotNum].trait.attributes = attributes;
    if (slotNum < 0 || slotNum >= obj->numSlots) {
        return 0;
    }
    return slotNum;
}



int ejsHasTrait(EjsObj *obj, int slotNum, int attributes)
{
    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        return 0;
    }
    return obj->slots[slotNum].trait.attributes & attributes;
}


int ejsGetTraitAttributes(EjsObj *obj, int slotNum)
{
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        mprAssert(0);
        return 0;
    }
    return obj->slots[slotNum].trait.attributes;
}


EjsType *ejsGetTraitType(EjsObj *obj, int slotNum)
{
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numSlots) {
        mprAssert(0);
        return 0;
    }
    return obj->slots[slotNum].trait.type;
}


int ejsRemoveProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    mprAssert(ejs);
    mprAssert(obj);
    
    if (slotNum < 0 || slotNum >= obj->numSlots) {
        return EJS_ERR;
    }
    removeSlot(ejs, (EjsObj*) obj, slotNum, 1);
    return 0;
}


/******************************* Hash Routines ********************************/
/*
    Exponential primes
 */
static int hashSizes[] = {
     19, 29, 59, 79, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 0
};


int ejsGetHashSize(int numSlots)
{
    int     i;

    for (i = 0; i < hashSizes[i]; i++) {
        if (numSlots < hashSizes[i]) {
            return hashSizes[i];
        }
    }
    return hashSizes[i - 1];
}


static int hashProperty(EjsObj *obj, int slotNum, EjsName *qname)
{
    EjsName     *slotName;
    int         chainSlotNum, lastSlot, index;

    mprAssert(qname);

    if (obj->hash == NULL || obj->hash->size < obj->numSlots) {
        /*  Remake the entire hash */
        return ejsMakeObjHash(obj);
    }
    index = ejsComputeHashCode(obj, qname);

    /* Scan the collision chain */
    lastSlot = -1;
    chainSlotNum = obj->hash->buckets[index];
    mprAssert(chainSlotNum < obj->numSlots);
    mprAssert(chainSlotNum < obj->sizeSlots);

    while (chainSlotNum >= 0) {
        slotName = &obj->slots[chainSlotNum].qname;
        if (CMP_QNAME(slotName, qname)) {
            return 0;
        }
        mprAssert(lastSlot != chainSlotNum);
        lastSlot = chainSlotNum;
        mprAssert(chainSlotNum != obj->slots[chainSlotNum].hashChain);
        chainSlotNum = obj->slots[chainSlotNum].hashChain;
        mprAssert(0 <= lastSlot && lastSlot < obj->sizeSlots);
    }
    if (lastSlot >= 0) {
        mprAssert(lastSlot < obj->numSlots);
        mprAssert(lastSlot != slotNum);
        obj->slots[lastSlot].hashChain = slotNum;

    } else {
        /* Start a new hash chain */
        obj->hash->buckets[index] = slotNum;
    }
    obj->slots[slotNum].hashChain = -2;
    obj->slots[slotNum].qname = *qname;
    return 0;
}


/*
    Allocate or grow the properties storage for an object. This routine will also manage the hash index for the object. 
    If numInstanceProp is < 0, then grow the number of properties by an increment. Otherwise, set the number of properties 
    to numInstanceProp. We currently don't allow reductions.
 */
int ejsMakeObjHash(EjsObj *obj)
{
    EjsSlot         *sp;
    EjsHash         *oldHash, *hp;
    int             i, newHashSize;

    mprAssert(obj);

    if (obj->numSlots <= EJS_HASH_MIN_PROP && obj->hash == 0) {
        /* Too few properties */
        return 0;
    }
    /*
        Reallocate the hash buckets if the hash needs to grow larger
     */
    oldHash = obj->hash;
    newHashSize = ejsGetHashSize(obj->numSlots);
    if (oldHash == NULL || oldHash->size < newHashSize) {
        mprFree(oldHash);
        hp = (EjsHash*) mprAlloc(obj, sizeof(EjsHash) + (newHashSize * sizeof(int)));
        if (hp == 0) {
            return EJS_ERR;
        }
        hp->buckets = (int*) (((char*) hp) + sizeof(EjsHash));
        hp->size = newHashSize;
        obj->hash = hp;

    }
    mprAssert(obj->hash);
    memset(obj->hash->buckets, -1, obj->hash->size * sizeof(int));

    /*
        Clear out hash linkage
     */
    if (oldHash) {
        for (sp = obj->slots, i = 0; i < obj->numSlots; i++, sp++) {
            sp->hashChain = -1;
        }
    }
    /*
        Rehash existing properties
     */
    for (sp = obj->slots, i = 0; i < obj->numSlots; i++, sp++) {
        if (sp->qname.name && hashProperty(obj, i, &sp->qname) < 0) {
            return EJS_ERR;
        }
    }
    return 0;
}


void ejsClearObjHash(EjsObj *obj)
{
    EjsSlot         *sp;
    int             i;

    mprAssert(obj);

    if (obj->hash) {
        memset(obj->hash->buckets, -1, obj->hash->size * sizeof(int));
        for (sp = obj->slots, i = 0; i < obj->numSlots; i++, sp++) {
            sp->hashChain = -1;
        }
    }
}


static void removeHashEntry(Ejs *ejs, EjsObj *obj, EjsName *qname)
{
    EjsSlot     *sp;
    EjsName     *nextName;
    int         index, slotNum, lastSlot, *buckets;

    if (obj->hash == 0) {
        /*
            No hash. Just do a linear search
         */
        for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
            sp = &obj->slots[slotNum];
            if (CMP_QNAME(&sp->qname, qname)) {
                sp->qname.name = "";
                sp->qname.space = "";
                sp->hashChain = -1;
                return;
            }
        }
        mprAssert(0);
        return;
    }
    index = ejsComputeHashCode(obj, qname);
    slotNum = obj->hash->buckets[index];
    lastSlot = -1;
    buckets = obj->hash->buckets;
    while (slotNum >= 0) {
        sp = &obj->slots[slotNum];
        nextName = &sp->qname;
        if (CMP_QNAME(nextName, qname)) {
            if (lastSlot >= 0) {
                obj->slots[lastSlot].hashChain = obj->slots[slotNum].hashChain;
            } else {
                buckets[index] = obj->slots[slotNum].hashChain;
            }
            sp->qname.name = "";
            sp->qname.space = "";
            sp->hashChain = -1;
            return;
        }
        lastSlot = slotNum;
        slotNum = obj->slots[slotNum].hashChain;
    }
    mprAssert(0);
}


/*
    Compute a property name hash. Based on work by Paul Hsieh.
 */
int ejsComputeHashCode(EjsObj *obj, EjsName *qname)
{
    uchar   *cdata;
    uint    len, hash, rem, tmp;

    mprAssert(obj);
    mprAssert(qname);
    mprAssert(qname->name);

    if ((len = (int) strlen(qname->name)) == 0) {
        return 0;
    }

    rem = len & 3;
    hash = len;

#if KEEP_FOR_UNICODE
    for (len >>= 2; len > 0; len--, data += 2) {
        hash  += *data;
        tmp   =  (data[1] << 11) ^ hash;
        hash  =  (hash << 16) ^ tmp;
        hash  += hash >> 11;
    }
#endif

    cdata = (uchar*) qname->name;
    for (len >>= 2; len > 0; len--, cdata += 4) {
        hash  += *cdata | (cdata[1] << 8);
        tmp   =  ((cdata[2] | (cdata[3] << 8)) << 11) ^ hash;
        hash  =  (hash << 16) ^ tmp;
        hash  += hash >> 11;
    }
    switch (rem) {
    case 3: 
        hash += cdata[0] + (cdata[1] << 8);
        hash ^= hash << 16;
        hash ^= cdata[sizeof(ushort)] << 18;
        hash += hash >> 11;
        break;
    case 2: 
        hash += cdata[0] + (cdata[1] << 8);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: hash += cdata[0];
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* 
        Force "avalanching" of final 127 bits 
     */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    mprAssert(obj->hash->size);
    return hash % obj->hash->size;
}


static int cmpName(EjsName *a, EjsName *b) 
{
    mprAssert(a);
    mprAssert(b);
    mprAssert(a->name);
    mprAssert(b->name);

    return (a->name == b->name || (a->name[0] == b->name[0] && strcmp(a->name, b->name) == 0));
}


static int cmpQname(EjsName *a, EjsName *b) 
{
    mprAssert(a);
    mprAssert(b);
    mprAssert(a->name);
    mprAssert(a->space);
    mprAssert(b->name);
    mprAssert(b->space);

    if (a->name == b->name && a->space == b->space) {
        return 1;
    }
    if (a->name[0] == b->name[0] && strcmp(a->name, b->name) == 0) {
        if (a->space[0] == b->space[0] && strcmp(a->space, b->space) == 0) {
            return 1;
        }
    }
    return 0;
}


int ejsCompactObject(Ejs *ejs, EjsObj *obj)
{
    EjsSlot     *slots, *src, *dest;
    int         i, removed;

    src = dest = slots = obj->slots;
    for (removed = i = 0; i < obj->numSlots; i++, src++) {
        if (src->value.ref == 0 || src->value.ref == ejs->undefinedValue || src->value.ref == ejs->nullValue) {
            removed++;
            continue;
        }
        *dest++ = *src;
    }
    obj->numSlots -= removed;
    ejsMakeObjHash(obj);
    return obj->numSlots;
}


/*********************************** Methods **********************************/
/*
    function get constructor(): Object
 */
static EjsObj *obj_constructor(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsObj      *constructor;
    EjsName     qname;

    if ((constructor = ejsGetPropertyByName(ejs, obj, ejsName(&qname, "", "constructor"))) != 0) {
        return constructor;
    }
    return (EjsObj*) obj->type;
}


/*
    function get prototype(): Object
 */
static EjsObj *obj_prototype(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsFunction     *fun;
    EjsType         *type;
    EjsObj          *prototype;

    if (ejs->compiling) {
        mprAssert(0);
        prototype = ejs->undefinedValue;
        
    } else if (ejsIsType(obj)) {
        prototype = ((EjsType*) obj)->prototype;
        
    } else if (ejsIsFunction(obj)) {
        fun = (EjsFunction*) obj;
        if (fun->archetype) {
            prototype = fun->archetype->prototype;
        
        } else {
            type = ejsCreateArchetype(ejs, fun, NULL);
            prototype = type->prototype;
        }
    } else {
        prototype = ejs->undefinedValue;
    }
    return prototype;
}


/*
    function set prototype(p: Object): Void
 */
static EjsObj *obj_set_prototype(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsObj          *prototype;
    EjsFunction     *fun;
    EjsName         pname;

    if (ejs->compiling) {
        mprAssert(0);
        return ejs->undefinedValue;
    }
    prototype = argv[0];
    if (ejsIsType(obj)) {
        ((EjsType*) obj)->prototype = prototype;
    } else {
        if (ejsIsFunction(obj)) {
            fun = (EjsFunction*) obj;
            if (ejsIsType(fun->archetype)) {
                fun->archetype->prototype = prototype;
            } else {
                ejsCreateArchetype(ejs, fun, prototype);
            }
        } else {
            /*
                Normal property creation. This "prototype" property is not used internally.
             */
            ejsSetPropertyByName(ejs, obj, ejsName(&pname, "", "prototype"), prototype);
        }
    }
    return 0;
}


/*
    function clone(deep: Boolean = true) : Object
 */
static EjsObj *obj_clone(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    bool    deep;

    deep = (argc == 1 && argv[0] == (EjsObj*) ejs->trueValue);
    return ejsClone(ejs, obj, deep);
}


/*
    static function create(proto: Object, props: Object = undefined): Void 
 */
static EjsObj *obj_create(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsFunction     *constructor;
    EjsObj          *obj, *properties, *options, *prototype;
    EjsType         *type;
    EjsName         qname;
    int             count, slotNum;

    prototype = argv[0];
    properties = (argc >= 1) ? argv[1] : 0;

    if (ejsIsType(prototype)) {
        type = (EjsType*) prototype;
    } else {
        constructor = ejsGetPropertyByName(ejs, prototype, ejsName(&qname, "", "constructor"));
        if (constructor) {
            if (ejsIsType(constructor)) {
                type = (EjsType*) constructor;
            } else if (ejsIsFunction(constructor)) {
                if (constructor->archetype == 0) {
                    if ((type = ejsCreateArchetype(ejs, constructor, prototype)) == 0) {
                        return 0;
                    }
                }
                type = constructor->archetype;
            } else {
                ejsThrowTypeError(ejs, "Bad type for the constructor property. Must be a function or type");
                return 0;
            }

        } else {
            if ((type = ejsCreateArchetype(ejs, NULL, prototype)) == 0) {
                return 0;
            }
            ejsSetPropertyByName(ejs, prototype, &qname, type);
        }
    }
    obj = ejsCreateObject(ejs, type, 0);
    if (properties) {
        count = ejsGetPropertyCount(ejs, properties);
        for (slotNum = 0; slotNum < count; slotNum++) {
            qname = ejsGetPropertyName(ejs, properties, slotNum);
            options = ejsGetProperty(ejs, properties, slotNum);
            argv[0] = obj;
            argv[1] = (EjsObj*) ejsCreateString(ejs, qname.name);
            argv[2] = options;
            obj_defineProperty(ejs, (EjsObj*) type, 3, argv);
        }
    }
    return (EjsObj*) obj;
}


/*
    static function defineProperty(obj: Object, prop: String, options: Object): Void
*/
static EjsObj *obj_defineProperty(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsFunction     *fun, *get, *set;
    EjsType         *type;
    EjsObj          *obj, *options, *configurable, *enumerable, *namespace, *value, *writable;
    EjsName         qname;
    char            *space, *name;
    int             attributes, slotNum;

    mprAssert(argc == 3);

    obj = argv[0];
    name = (char*) mprStrdup(obj, ejsGetString(ejs, argv[1]));
    options = argv[2];
    space = (char*) "";
    value = 0;
    set = get = 0;
    attributes = 0;

    if ((namespace = ejsGetPropertyByName(ejs, options, EN(&qname, "namespace"))) != 0) {
        space = (char*) mprStrdup(ejs, ejsGetString(ejs, namespace));
    }
    if ((slotNum = ejsLookupProperty(ejs, obj, ejsName(&qname, space, name))) >= 0) {
        if (ejsHasTrait(obj, slotNum, EJS_TRAIT_FIXED)) {
            ejsThrowTypeError(ejs, "Property \"%s\" is not configurable", qname.name);
            return 0;
        }
    }
    type = ejsGetPropertyByName(ejs, options, EN(&qname, "type"));

    if ((configurable = ejsGetPropertyByName(ejs, options, EN(&qname, "configurable"))) != 0) {
        if (configurable == (EjsObj*) ejs->falseValue) {
            attributes |= EJS_TRAIT_FIXED;
        }
    }
    if ((enumerable = ejsGetPropertyByName(ejs, options, EN(&qname, "enumerable"))) != 0) {
        if (enumerable == (EjsObj*) ejs->falseValue) {
            attributes |= EJS_TRAIT_HIDDEN;
        }
    }
    value = ejsGetPropertyByName(ejs, options, EN(&qname, "value"));
    if (value && type && !ejsIsA(ejs, value, type)) {
        ejsThrowArgError(ejs, "Value is not of the required type");
        return 0;
    }

    if ((get = ejsGetPropertyByName(ejs, options, EN(&qname, "get"))) != 0) {
        if (ejsIsFunction(get)) {
            get->setter = ejsGetPropertyByName(ejs, obj, ejsName(&qname, space, "set"));
            attributes |= EJS_TRAIT_GETTER;
        } else {
            ejsThrowArgError(ejs, "The \"get\" property is not a function");
            return 0;
        }
    }
    if ((set = ejsGetPropertyByName(ejs, options, EN(&qname, "set"))) != 0) {
        if (ejsIsFunction(set)) {
            if (get == 0 && (fun = ejsGetPropertyByName(ejs, obj, ejsName(&qname, space, name))) != 0) {
                get = fun;
            }
            if (get) {
                get->setter = set;
            }
        } else {
            ejsThrowArgError(ejs, "The \"set\" property is not a function");
            return 0;
        }
        attributes |= EJS_TRAIT_SETTER;
    }
    if (value && (get || set)) {
        ejsThrowArgError(ejs, "Can't provide a value and getters or setters");
        return 0;
    }
    if (get) {
        value = (EjsObj*) get;
    }
    if ((writable = ejsGetPropertyByName(ejs, options, EN(&qname, "writable"))) != 0) {
        if (writable == (EjsObj*) ejs->falseValue) {
            attributes |= EJS_TRAIT_READONLY;
        }
    }
    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);
    ejsName(&qname, space, name);
    if (defineObjectProperty(ejs, obj, -1, ejsName(&qname, space, name), type, attributes, value) < 0) {
        ejsThrowTypeError(ejs, "Can't define property %s", name);
    }
    return 0;
}


/*
    static function freeze(obj: Object): Void
 */
static EjsObj *obj_freeze(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    int         slotNum;

    obj = argv[0];
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        obj->slots[slotNum].trait.attributes |= EJS_TRAIT_READONLY | EJS_TRAIT_FIXED;
    }
    if (ejsIsType(obj)) {
        obj = ((EjsType*) obj)->prototype;
        for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
            obj->slots[slotNum].trait.attributes |= EJS_TRAIT_READONLY | EJS_TRAIT_FIXED;
        }
    }
    obj->dynamic = 0;
    return 0;
}


/*
    Function to iterate and return the next element name.
    NOTE: this is not a method of Object. Rather, it is a callback function for Iterator.
 */
static EjsObj *nextObjectKey(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    EjsName     qname;
    EjsTrait    *trait;

    obj = (EjsObj*) ip->target;

    for (; ip->index < obj->numSlots; ip->index++) {
        qname = ejsGetPropertyName(ejs, (EjsObj*) obj, ip->index);
        if (qname.name[0] == '\0') {
            continue;
        }
        trait = ejsGetTrait(ejs, obj, ip->index);
        if (trait && trait->attributes & 
            /* MOB OPT - make initializers and deleted items always hidden */
                (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
            continue;
        }
        ip->index++;
        return (EjsObj*) ejsCreateString(ejs, qname.name);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator.

    iterator function get(options: Object = null): Iterator
 */
static EjsObj *obj_get(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, obj, (EjsProc) nextObjectKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Object. Rather, it is a callback function for Iterator
 */
static EjsObj *nextObjectValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    EjsTrait    *trait;

    obj = (EjsObj*) ip->target;

    for (; ip->index < obj->numSlots; ip->index++) {
        trait = ejsGetTrait(ejs, obj, ip->index);
        if (trait && trait->attributes & 
                (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
            continue;
        }
        return obj->slots[ip->index++].value.ref;
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator function getValues(options: Object = null): Iterator
 */
static EjsObj *obj_getValues(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, obj, (EjsProc) nextObjectValue, 0, NULL);
}


/*
    Get the number of properties in the object.

    function get getOwnPropertyCount(obj): Number
 */
static EjsObj *obj_getOwnPropertyCount(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return (EjsObj*) ejsCreateNumber(ejs, ejsGetPropertyCount(ejs, obj) - obj->type->numInherited);
}


/*
    static function getOwnPropertyDescriptor(obj: Object, prop: String): Object
 */
static EjsObj *obj_getOwnPropertyDescriptor(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsFunction     *fun;
    EjsTrait        *trait;
    EjsObj          *obj, *result, *value;
    EjsName         qname, qn;
    EjsType         *type;
    EjsLookup       lookup;
    cchar           *prop;
    int             slotNum;

    obj = argv[0];
    prop = ejsGetString(ejs, argv[1]);
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, EN(&qname, prop), &lookup)) < 0) {
        return (EjsObj*) ejs->falseValue;
    }
    trait = ejsGetTrait(ejs, obj, slotNum);
    result = ejsCreateSimpleObject(ejs);
    value = ejsGetVarByName(ejs, obj, EN(&qname, prop), &lookup);
    if (value == 0) {
        value = ejs->nullValue;
    }
    type = (trait) ? trait->type: 0;
    if (trait && trait->attributes & EJS_TRAIT_GETTER) {
        ejsSetPropertyByName(ejs, result, EN(&qname, "get"), value);
    } else if (trait && trait->attributes & EJS_TRAIT_SETTER) {
        fun = (EjsFunction*) value;
        if (ejsIsFunction(fun)) {
            ejsSetPropertyByName(ejs, result, EN(&qname, "set"), fun->setter);
        }
    } else if (value) {
        ejsSetPropertyByName(ejs, result, EN(&qname, "value"), value);
    }
    ejsSetPropertyByName(ejs, result, EN(&qname, "configurable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_FIXED)));
    ejsSetPropertyByName(ejs, result, EN(&qname, "enumerable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_HIDDEN)));
    qn = ejsGetPropertyName(ejs, obj, slotNum);
    ejsSetPropertyByName(ejs, result, EN(&qname, "namespace"), ejsCreateString(ejs, qn.space));
    ejsSetPropertyByName(ejs, result, EN(&qname, "type"), 
         type ? (EjsObj*) type : ejs->nullValue);
    ejsSetPropertyByName(ejs, result, EN(&qname, "writable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_READONLY)));
    return result;
}


/*
    Get all properties names including non-enumerable properties

    static function getOwnPropertyNames(obj: Object, options: Object): Array
 */
static EjsObj *obj_getOwnPropertyNames(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj, *options, *arg;
    EjsArray    *result;
    EjsTrait    *trait;
    EjsName     qname;
    int         slotNum, index, includeBases, excludeFunctions;

    obj = argv[0];
    includeBases = 0;
    excludeFunctions = 0;
    if (argc > 0) {
        options = argv[1];
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "includeBases"))) != 0) {
            includeBases = (arg == (EjsObj*) ejs->trueValue);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "excludeFunctions"))) != 0) {
            excludeFunctions = (arg == (EjsObj*) ejs->trueValue);
        }
    }
    if ((result = ejsCreateArray(ejs, 0)) == 0) {
        return 0;
    }
    index = 0;
    slotNum = (includeBases) ? 0 : obj->type->numInherited;
    for (; slotNum < obj->numSlots; slotNum++) {
        if ((trait = ejsGetTrait(ejs, obj, slotNum)) != 0) {
            if (trait->attributes & (EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
#if UNUSED
        if (qname.name[0] == '\0') {
            continue;
        }
#endif
        if (excludeFunctions && ejsIsFunction(ejsGetProperty(ejs, obj, slotNum))) {
            continue;
        }
        ejsSetProperty(ejs, result, index++, ejsCreateString(ejs, qname.name));
    }
    if (ejsIsType(obj) || ejsIsFunction(obj)) {
        if (ejsLookupProperty(ejs, obj, ejsName(&qname, "", "prototype")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateString(ejs, "prototype"));
        }
        if (ejsLookupProperty(ejs, obj, ejsName(&qname, "", "length")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateString(ejs, "length"));
        }
    } else if (obj->isPrototype) {
        if (ejsLookupProperty(ejs, obj, ejsName(&qname, "", "constructor")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateString(ejs, "constructor"));
        }
    }
    return (EjsObj*) result;
}


/*
    static function getOwnPrototypeOf(obj: Object): Type
 */
static EjsObj *obj_getOwnPrototypeOf(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return (EjsObj*) obj->type->prototype;
}


/*
    function hasOwnProperty(name: String): Boolean
 */
static EjsObj *obj_hasOwnProperty(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsName     qname;
    EjsLookup   lookup;
    int         slotNum;
    cchar       *prop;

    prop = ejsGetString(ejs, argv[0]);
    slotNum = ejsLookupVarWithNamespaces(ejs, obj, EN(&qname, prop), &lookup);
    return (EjsObj*) ejsCreateBoolean(ejs, slotNum >= 0);
}


/*
    static function isExtensible(obj: Object): Boolean
 */
static EjsObj *obj_isExtensible(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return (EjsObj*) ejsCreateBoolean(ejs, obj->dynamic);
}


/*
    static function isFrozen(obj: Object): Boolean
 */
static EjsObj *obj_isFrozen(Ejs *ejs, EjsObj *type, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsObj      *obj;
    int         frozen, slotNum;

    obj = argv[0];
    frozen = 1;
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        if ((trait = ejsGetTrait(ejs, obj, slotNum)) != 0) {
            if (!(trait->attributes & EJS_TRAIT_READONLY)) {
                frozen = 0;
                break;
            }
            if (!(trait->attributes & EJS_TRAIT_FIXED)) {
                frozen = 0;
                break;
            }
        }
    }
    if (obj->dynamic) {
        frozen = 0;
    }
    return (EjsObj*) ejsCreateBoolean(ejs, frozen);
}


/*
    static function isPrototypeOf(obj: Object): Boolean
 */
static EjsObj *obj_isPrototypeOf(Ejs *ejs, EjsObj *prototype, int argc, EjsObj **argv)
{
    EjsObj  *obj;
    
    obj = argv[0];
    return prototype == obj->type->prototype ? ejs->trueValue : ejs->falseValue;
}


/*
    static function isSealed(obj: Object): Boolean
 */
static EjsObj *obj_isSealed(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsObj      *obj;
    int         sealed, slotNum;

    obj = argv[0];
    sealed = 1;
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        if ((trait = ejsGetTrait(ejs, obj, slotNum)) != 0) {
            if (!(trait->attributes & EJS_TRAIT_FIXED)) {
                sealed = 0;
                break;
            }
        }
    }
    if (obj->dynamic) {
        sealed = 0;
    }
    return (EjsObj*) ejsCreateBoolean(ejs, sealed);
}


#if FUTURE
/*
    Get enumerable properties names

    static function keys(obj: Object): Array
 */
static EjsObj *obj_keys(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj, *vp;
    EjsArray    *result;
    EjsTrait    *trait;
    EjsName     qname;
    int         slotNum;

    obj = argv[0];
    if ((result = ejsCreateArray(ejs, obj->numSlots)) == 0) {
        return 0;
    }
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        if ((trait = ejsGetTrait(ejs, obj, slotNum)) != 0) {
            if (trait->attributes & EJS_TRAIT_DELETED) {
                continue;
            }
        }
        vp = ejsGetProperty(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        ejsSetProperty(ejs, result, slotNum, ejsCreateString(ejs, qname.name));
    }
    return (EjsObj*) result;
}
#endif


/*
    static function preventExtensions(obj: Object): Object
 */
static EjsObj *obj_preventExtensions(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    obj->dynamic = 0;
    return obj;
}


/*
    static function seal(obj: Object): Void
*/
static EjsObj *obj_seal(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsObj      *obj;
    int         slotNum;

    obj = argv[0];
    for (slotNum = 0; slotNum < obj->numSlots; slotNum++) {
        //  MOB -- API confused
        trait = ejsGetTrait(ejs, obj, slotNum);
        trait->attributes |= EJS_TRAIT_FIXED;
    }
    obj->dynamic = 0;
    return 0;
}


/*
    function propertyIsEnumerable(property: String, flag: Object = undefined): Boolean
 */
static EjsObj *obj_propertyIsEnumerable(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsName     qname;
    EjsLookup   lookup;
    cchar       *prop;
    int         slotNum;

    mprAssert(argc == 1 || argc == 2);

    prop = ejsGetString(ejs, argv[0]);
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, EN(&qname, prop), &lookup)) < 0) {
        return (EjsObj*) ejs->falseValue;
    }
    trait = ejsGetTrait(ejs, obj, slotNum);
    return (EjsObj*) ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_HIDDEN));
}


/*
    Convert the object to a JSON string. This also handles Json for Arrays.

    function toJSON(options: Object = null): String

    Options: baseClasses, depth, indent, hidden, pretty, replacer
 */
static EjsObj *obj_toJSON(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    MprBuf      *buf;
    EjsObj      *arg, *pp, *result;
    EjsName     qname;
    EjsObj      *obj, *options, *replacerArgs[2];
    EjsFunction *replacer;
    EjsString   *sv;
    EjsTrait    *trait;
    char        key[16], *cp, *indent;
    int         c, isArray, i, count, slotNum, depth, hidden, baseClasses, namespaces, pretty;

    /*
        The main code below can handle Arrays, Objects, objects derrived from Object and also native classes with properties.
        All others just use toString
     */
    count = ejsGetPropertyCount(ejs, vp);
    if (count == 0 && vp->type != ejs->objectType && vp->type != ejs->arrayType) {
        return (EjsObj*) ejsToString(ejs, vp);
    }
    depth = 99;
    baseClasses = hidden = namespaces = pretty = 0;
    replacer = NULL;
    indent = NULL;
    options = NULL;
    
    if (argc > 0) {
        options = argv[0];
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "baseClasses"))) != 0) {
            baseClasses = (arg == (EjsObj*) ejs->trueValue);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "depth"))) != 0) {
            depth = ejsGetInt(ejs, arg);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "indent"))) != 0) {
            if (ejsIsString(arg)) {
               indent = ((EjsString*) arg)->value; 
            } else if (ejsIsNumber(arg)) {
                i = ejsGetInt(ejs, arg);
                if (i < 0 || i >= MPR_MAX_STRING) {
                    indent = NULL;
                } else {
                    indent = mprAlloc(arg, i + 1);
                    memset(indent, ' ', i);
                    indent[i] = '\0';
                }
            }
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "hidden"))) != 0) {
            hidden = (arg == (EjsObj*) ejs->trueValue);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "namespaces"))) != 0) {
            namespaces = (arg == (EjsObj*) ejs->trueValue);
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN(&qname, "pretty"))) != 0) {
            pretty = (arg == (EjsObj*) ejs->trueValue);
        }
        replacer = ejsGetPropertyByName(ejs, options, EN(&qname, "replacer"));
        if (!ejsIsFunction(replacer)) {
            replacer = NULL;
        }
    }
    isArray = ejsIsArray(vp);

    obj = (EjsObj*) vp;
    buf = mprCreateBuf(ejs, 0, 0);
    mprPutCharToBuf(buf, isArray ? '[' : '{');
    if (pretty || indent) {
        mprPutCharToBuf(buf, '\n');
    }
    if (++ejs->serializeDepth <= depth && !obj->visited) {
        obj->visited = 1;
        for (slotNum = 0; slotNum < count && !ejs->exception; slotNum++) {
            trait = ejsGetTrait(ejs, obj, slotNum);
            if (trait && (trait->attributes & (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | 
                    EJS_FUN_MODULE_INITIALIZER)) && !hidden) {
                continue;
            }
            pp = ejsGetProperty(ejs, obj, slotNum);
            if (ejs->exception) {
                obj->visited = 0;
                return 0;
            }
            if (pp == 0) {
                continue;
            }
            if (isArray) {
                mprItoa(key, sizeof(key), slotNum, 10);
                qname.name = key;
                qname.space = "";
            } else {
                qname = ejsGetPropertyName(ejs, vp, slotNum);
            }
            if (qname.name == 0) {
                continue;
            }
            if (qname.space[0] == '\0' && qname.name[0] == '\0') {
                continue;
            }
            if (pretty) {
                for (i = 0; i < ejs->serializeDepth; i++) {
                    mprPutStringToBuf(buf, "  ");
                }
            } else {
                if (indent) {
                    mprPutStringToBuf(buf, indent);
                }
            }
            if (!isArray) {
                if (namespaces) {
                    if (qname.space[0] != EJS_EMPTY_NAMESPACE[0]) {
                        mprPutFmtToBuf(buf, "\"%s\"::", qname.space);
                    }
                }
                mprPutCharToBuf(buf, '"');
                for (cp = (char*) qname.name; cp && *cp; cp++) {
                    c = *cp;
                    if (c == '"' || c == '\\') {
                        mprPutCharToBuf(buf, '\\');
                        mprPutCharToBuf(buf, c);
                    } else {
                        mprPutCharToBuf(buf, c);
                    }
                }
                mprPutStringToBuf(buf, "\":");
                if (pretty || indent) {
                    mprPutCharToBuf(buf, ' ');
                }
            }
//  MOB GC - Can this be collected?
            sv = (EjsString*) ejsToJSON(ejs, pp, options);
            if (sv == 0 || !ejsIsString(sv)) {
                if (!ejs->exception) {
                    ejsThrowTypeError(ejs, "Can't serialize property %s", qname.name);
                }
                obj->visited = 0;
                return 0;
            } else {
                if (replacer) {
//  MOB GC - Can this be collected?
                    replacerArgs[0] = (EjsObj*) ejsCreateString(ejs, qname.name); 
                    replacerArgs[1] = (EjsObj*) sv; 
                    pp = ejsRunFunction(ejs, replacer, obj, 2, replacerArgs);
                }
                mprPutStringToBuf(buf, sv->value);
            }
            if ((slotNum + 1) < count) {
                mprPutCharToBuf(buf, ',');
            }
            if (pretty || indent) {
                mprPutCharToBuf(buf, '\n');
            }
        }
        obj->visited = 0;
    }
    --ejs->serializeDepth; 
    if (pretty || indent) {
        for (i = ejs->serializeDepth; i > 0; i--) {
            mprPutStringToBuf(buf, "  ");
        }
    }
    mprPutCharToBuf(buf, isArray ? ']' : '}');
    mprAddNullToBuf(buf);
//  MOB GC
    result = (EjsObj*) ejsCreateString(ejs, mprGetBufStart(buf));
    mprFree(buf);
    return result;
}


#if ES_Object_toLocaleString
/*
    Convert the object to a localized string

    function toLocaleString(): String
 */
static EjsObj *toLocaleString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return obj_toString(ejs, vp, argc, argv);
}
#endif


static EjsObj *obj_toString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    if (ejsIsString(vp)) {
        return vp;
    }
    return (EjsObj*) castObject(ejs, vp, ejs->stringType);
}


/************************************************** Reflection **********************************************/
/*
    Get the base class of the object.

    function getBaseType(obj: Type): Type
 */
static EjsObj *obj_getBaseType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = argv[0];
    if (ejsIsType(vp)) {
        return (EjsObj*) (((EjsType*) vp)->baseType);
    }
    return (EjsObj*) ejs->nullValue;
}


/*
    function isPrototype(obj: Object): Boolean
 */
static EjsObj *obj_isPrototype(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, ejsIsPrototype(argv[0]));
}


/*
    function isType(obj: Object): Boolean
 */
static EjsObj *obj_isType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, ejsIsType(argv[0]));
}


/*
    Get the type of the object.

    function getType(obj: Object): Type
 */
static EjsObj *obj_getType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = argv[0];
    if (vp->type == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) vp->type;
}

/*
    Return the type name of a var as a string. If the var is a type, get the base type.
 */
EjsObj *ejsGetTypeName(Ejs *ejs, EjsObj *vp)
{
    EjsType     *type;

    if (vp == 0) {
        return ejs->undefinedValue;
    }
    type = (EjsType*) vp->type;
    if (type == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, type->qname.name);
}


/*
    function getTypeName(obj): String
 */
static EjsObj *obj_getTypeName(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return (EjsObj*) ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the name of a function or type object

    function getName(obj: Object): String
 */
static EjsObj *obj_getName(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];

    if (ejsIsType(obj)) {
        return (EjsObj*) ejsCreateString(ejs, ((EjsType*) obj)->qname.name);
    } else if (ejsIsFunction(obj)) {
        return (EjsObj*) ejsCreateString(ejs, ((EjsFunction*) obj)->name);
    }
    return (EjsObj*) ejs->emptyStringValue;
}

/*********************************** Globals **********************************/

/*
    function typeOf(obj): String
 */
static EjsObj *obj_typeOf(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return (EjsObj*) ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the ecma "typeof" value for an object. Unfortunately, typeof is pretty lame.
 */
EjsObj *ejsGetTypeOf(Ejs *ejs, EjsObj *vp)
{
    if (vp == ejs->undefinedValue) {
        return (EjsObj*) ejsCreateString(ejs, "undefined");

    } else if (vp == ejs->nullValue) {
        /* Yea - I know, ECMAScript is broken */
        return (EjsObj*) ejsCreateString(ejs, "object");

    } if (ejsIsBoolean(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "boolean");

    } else if (ejsIsNumber(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "number");

    } else if (ejsIsString(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "string");

    } else if (ejsIsFunction(vp)) {
        return (EjsObj*) ejsCreateString(ejs, "function");
               
    } else if (ejsIsType(vp)) {
        /* Pretend it is a constructor function */
        return (EjsObj*) ejsCreateString(ejs, "function");
               
    } else {
        return (EjsObj*) ejsCreateString(ejs, "object");
    }
}


/************************************ Factory *********************************/

void ejsCreateObjectHelpers(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;

    type = ejs->objectType;
    helpers = &type->helpers;
    helpers->cast                   = (EjsCastHelper) castObject;
    helpers->clone                  = (EjsCloneHelper) ejsCloneObject;
    helpers->create                 = (EjsCreateHelper) ejsCreateObject;
    helpers->defineProperty         = (EjsDefinePropertyHelper) defineObjectProperty;
    helpers->deleteProperty         = (EjsDeletePropertyHelper) deleteObjectProperty;
    helpers->deletePropertyByName   = (EjsDeletePropertyByNameHelper) deleteObjectPropertyByName;
    helpers->destroy                = (EjsDestroyHelper) destroyObject;
    helpers->getProperty            = (EjsGetPropertyHelper) getObjectProperty;
    helpers->getPropertyCount       = (EjsGetPropertyCountHelper) getObjectPropertyCount;
    helpers->getPropertyName        = (EjsGetPropertyNameHelper) getObjectPropertyName;
    helpers->lookupProperty         = (EjsLookupPropertyHelper) lookupObjectProperty;
    helpers->invokeOperator         = (EjsInvokeOperatorHelper) ejsObjectOperator;
    helpers->mark                   = (EjsMarkHelper) ejsMarkObject;
    helpers->setProperty            = (EjsSetPropertyHelper) setObjectProperty;
    helpers->setPropertyName        = (EjsSetPropertyNameHelper) setObjectPropertyName;
#if UNUSED
    helpers->getPropertyTrait       = (EjsGetPropertyTraitHelper) getObjectPropertyTrait;
    helpers->setPropertyTrait       = (EjsSetPropertyTraitHelper) setObjectPropertyTrait;
#endif
}


void ejsConfigureObjectType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;
    EjsFunction *fun;

    type = ejsGetTypeByName(ejs, "ejs", "Object");
    prototype = type->prototype;

    ejsBindMethod(ejs, type, ES_Object_create, obj_create);
    ejsBindMethod(ejs, type, ES_Object_defineProperty, obj_defineProperty);
    ejsBindMethod(ejs, type, ES_Object_freeze, obj_freeze);
    ejsBindMethod(ejs, type, ES_Object_getOwnPropertyCount, obj_getOwnPropertyCount);
    ejsBindMethod(ejs, type, ES_Object_getOwnPropertyDescriptor, obj_getOwnPropertyDescriptor);
    ejsBindMethod(ejs, type, ES_Object_getOwnPropertyNames, obj_getOwnPropertyNames);
    ejsBindMethod(ejs, type, ES_Object_getOwnPrototypeOf, obj_getOwnPrototypeOf);
    ejsBindMethod(ejs, type, ES_Object_isExtensible, obj_isExtensible);
    ejsBindMethod(ejs, type, ES_Object_isFrozen, obj_isFrozen);
    ejsBindMethod(ejs, type, ES_Object_isSealed, obj_isSealed);
    ejsBindMethod(ejs, type, ES_Object_preventExtensions, obj_preventExtensions);
    ejsBindAccess(ejs, type, ES_Object_prototype, (EjsProc) obj_prototype, obj_set_prototype);
    ejsBindMethod(ejs, type, ES_Object_seal, obj_seal);

    /* Reflection */
    ejsBindMethod(ejs, type, ES_Object_getBaseType, obj_getBaseType);
    ejsBindMethod(ejs, type, ES_Object_getType, obj_getType);
    ejsBindMethod(ejs, type, ES_Object_getTypeName, obj_getTypeName);
    ejsBindMethod(ejs, type, ES_Object_getName, obj_getName);
    ejsBindMethod(ejs, type, ES_Object_isType, obj_isType);
    ejsBindMethod(ejs, type, ES_Object_isPrototype, obj_isPrototype);

    ejsBindMethod(ejs, prototype, ES_Object_constructor, (EjsProc) obj_constructor);
    ejsBindMethod(ejs, prototype, ES_Object_clone, obj_clone);
    ejsBindMethod(ejs, prototype, ES_Object_iterator_get, obj_get);
    ejsBindMethod(ejs, prototype, ES_Object_iterator_getValues, obj_getValues);
    ejsBindMethod(ejs, prototype, ES_Object_hasOwnProperty, obj_hasOwnProperty);
    ejsBindMethod(ejs, prototype, ES_Object_isPrototypeOf, obj_isPrototypeOf);
    ejsBindMethod(ejs, prototype, ES_Object_propertyIsEnumerable, obj_propertyIsEnumerable);
    ejsBindMethod(ejs, prototype, ES_Object_toLocaleString, toLocaleString);
    ejsBindMethod(ejs, prototype, ES_Object_toString, obj_toString);
    ejsBindMethod(ejs, prototype, ES_Object_toJSON, obj_toJSON);

    ejsBindFunction(ejs, ejs->globalBlock, ES_typeOf, (EjsProc) obj_typeOf);

    /*
        The prototype method is special. It is declared as static so it is generated in the type slots, but it is
        patched to be an instance method so the value of "this" will be preserved when it is invoked.
     */
    fun = ejsGetProperty(ejs, (EjsObj*) type, ES_Object_prototype);
    fun->staticMethod = 0;
    fun->setter->staticMethod = 0;
    type->constructor.block.obj.slots[ES_Object_prototype].trait.attributes &= ~EJS_PROP_STATIC;
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
