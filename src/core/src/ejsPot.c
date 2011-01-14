/**
    ejsPot.c - Property Object class (Objects with properties)

    This is the base for all scripted classes.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Defines **********************************/

#define CMP_QNAME(a,b) ((a)->name == (b)->name && (a)->space == (b)->space)
#define CMP_NAME(a,b) ((a)->name == (b)->name)

/****************************** Forward Declarations **************************/

static int  growSlots(Ejs *ejs, EjsPot *obj, int size);
static int  hashProperty(Ejs *ejs, EjsPot *obj, int slotNum, EjsName qname);
static void removeHashEntry(Ejs *ejs, EjsPot *obj, EjsName qname);

/************************************* Code ***********************************/

EjsAny *ejsCreateEmptyPot(Ejs *ejs)
{
    return ejsCreatePot(ejs, ejs->objectType, 0);
}


EjsAny *ejsClonePot(Ejs *ejs, EjsAny *vp, bool deep)
{
    EjsPot      *dest, *src;
    EjsSlot     *dp, *sp;
    int         numProp, i;

    if (!ejsIsPot(ejs, vp)) {
        mprAssert(ejsIsPot(ejs, vp));
        return NULL;
    }
    src = (EjsPot*) vp;
    numProp = src->numProp;
    if ((dest = ejsCreatePot(ejs, TYPE(src), numProp)) == 0) {
        return 0;
    }
    dest->obj = src->obj;
    dest->isBlock = src->isBlock;
    dest->isFrame = src->isFrame;
    dest->isFunction = src->isFunction;
    dest->isPrototype = src->isPrototype;
    dest->isType = src->isType;
    dest->numProp = numProp;
    dest->shortScope = src->shortScope;

    dp = dest->properties->slots;
    sp = src->properties->slots;
    for (i = 0; i < numProp; i++, sp++, dp++) {
        *dp = *sp;
        dp->hashChain = -1;
        if (deep && !TYPE(sp->value.ref)->immutable) {
            dp->value.ref = ejsClone(ejs, sp->value.ref, deep);
        }
    }
    if (dest->numProp > EJS_HASH_MIN_PROP) {
        ejsMakeHash(ejs, dest);
    }
    ejsCopyName(dest, src);
    return dest;
}


static EjsObj *prepareAccessors(Ejs *ejs, EjsObj *obj, int slotNum, int64 *attributes, EjsObj *value)
{
    EjsFunction     *fun;
    EjsTrait        *trait;

    fun = ejsGetProperty(ejs, obj, slotNum);

    if (*attributes & EJS_TRAIT_SETTER) {
        if (ejsIsFunction(ejs, fun)) {
            /* Existing getter, add a setter */
            fun->setter = (EjsFunction*) value;
            if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
                *attributes |= trait->attributes;
            }
        } else {
            /* No existing getter, must define a dummy getter - will not be called */
            fun = (EjsFunction*) ejsCloneFunction(ejs, ejs->nopFunction, 0);
            fun->setter = (EjsFunction*) value;
        }
        value = (EjsObj*) fun;

    } else if (*attributes & EJS_TRAIT_GETTER) {
        if (ejsIsFunction(ejs, fun) && ejsPropertyHasTrait(ejs, obj, slotNum, EJS_TRAIT_SETTER)) {
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
static int definePotProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsName qname, EjsType *propType, int64 attributes, 
    EjsObj *value)
{
    EjsFunction     *fun;
    EjsType         *type;
    int             priorSlot;

    mprAssert(ejs);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= -1);

    if (attributes & (EJS_TRAIT_GETTER | EJS_TRAIT_SETTER) && !ejsIsFunction(ejs, value)) {
        ejsThrowTypeError(ejs, "Property \"%@\" is not a function", qname.name);
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
    if (ejsSetPropertyTraits(ejs, (EjsObj*) obj, slotNum, propType, (int) attributes) < 0) {
        return EJS_ERR;
    }

    //  MOB -- reconsider this code
    if (value && ejsIsFunction(ejs, value)) {
        fun = ((EjsFunction*) value);
        if (!ejsIsNativeFunction(ejs, fun) && ejsIsType(ejs, obj)) {
            ((EjsType*) obj)->hasScriptFunctions = 1;
        }
        if (fun->staticMethod && ejsIsType(ejs, obj)) {
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
static int deletePotProperty(Ejs *ejs, EjsPot *obj, int slotNum)
{
    EjsName     qname;
    EjsSlot     *sp;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numProp) {
        ejsThrowReferenceError(ejs, "Invalid property slot to delete");
        return EJS_ERR;
    }
#if UNUSED
    //  MOB -- this should be in the VM and not here
    if (!DYNAMIC(obj)) {
        //  MOB -- probably can remove this and rely on fixed below as per ecma spec
        ejsThrowTypeError(ejs, "Can't delete properties in a non-dynamic object");
        return EJS_ERR;
    } else if (ejsPropertyHasTrait(ejs, obj, slotNum, EJS_TRAIT_FIXED)) {
        ejsThrowTypeError(ejs, "Property \"%@\" is not deletable", qname.name);
        return EJS_ERR;
    }
#endif
    qname = ejsGetPotPropertyName(ejs, obj, slotNum);
    if (qname.name) {
        removeHashEntry(ejs, obj, qname);
    }
    sp = &obj->properties->slots[slotNum];
    sp->value.ref = ejs->undefinedValue;
    sp->trait.type = 0;
    sp->trait.attributes = EJS_TRAIT_DELETED | EJS_TRAIT_HIDDEN;
    return 0;
}


static int deletePotPropertyByName(Ejs *ejs, EjsPot *obj, EjsName qname)
{
    int     slotNum;

    mprAssert(ejsIsPot(ejs, obj));

    slotNum = ejsLookupPotProperty(ejs, obj, qname);
    if (slotNum < 0) {
        ejsThrowReferenceError(ejs, "Property does not exist");
        return EJS_ERR;
    }
    return deletePotProperty(ejs, obj, slotNum);
}


static EjsPot *getPotProperty(Ejs *ejs, EjsPot *obj, int slotNum)
{
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numProp) {
        ejsThrowReferenceError(ejs, "Property at slot \"%d\" is not found", slotNum);
        return 0;
    }
    return obj->properties->slots[slotNum].value.ref;
}


static int getPotPropertyCount(Ejs *ejs, EjsPot *obj)
{
    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));

    return obj->numProp;
}


EjsName ejsGetPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum)
{
    EjsName     qname;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numProp) {
        qname.name = 0;
        qname.space = 0;
        return qname;
    }
    return obj->properties->slots[slotNum].qname;
}


#if UNUSED
static EjsTrait *getPotPropertyTrait(Ejs *ejs, EjsPot *obj, int slotNum)
{
    mprAssert(ejsIsPot(ejs, obj));
    return ejsGetPropertyTraits(ejs, obj, slotNum);
}
#endif


/*
    Lookup a property with a namespace qualifier in an object and return the slot if found. Return EJS_ERR if not found.
    If qname.space is NULL, then only return a positive slot if there is only one property of the given name.
    Only the name portion is hashed. The namespace is not included in the hash. This is used to do a one-step lookup 
    for properties regardless of the namespace.
 */
int ejsLookupPotProperty(struct Ejs *ejs, EjsPot *obj, EjsName qname)
{
    EjsProperties   *props;
    EjsHash         *hash;
    EjsSlot         *slots, *sp, *np;
    int             slotNum, index, prior;

    mprAssert(qname.name);
    mprAssert(ejsIsPot(ejs, obj));

    if ((props = obj->properties) == 0 || obj->numProp == 0) {
        return -1;
    }
    slots = props->slots;
    if ((hash = props->hash) == 0 || hash->size == 0) {
        /* No hash. Just do a linear search */
        if (qname.space) {
            for (slotNum = 0; slotNum < obj->numProp; slotNum++) {
                sp = &slots[slotNum];
                if (CMP_QNAME(&sp->qname, &qname)) {
                    return slotNum;
                }
            }
            return -1;
        } else {
            for (slotNum = 0, prior = -1; slotNum < obj->numProp; slotNum++) {
                sp = &slots[slotNum];
                if (CMP_NAME(&sp->qname, &qname)) {
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
        mprAssert(props->hash);
        mprAssert(props->hash->size > 0);
        index = whash(qname.name->value, qname.name->length) % props->hash->size;
        if (qname.space) {
            mprAssert(hash->buckets);
            mprAssert(index < hash->size);
            for (slotNum = hash->buckets[index]; slotNum >= 0; slotNum = slots[slotNum].hashChain) {
                sp = &slots[slotNum];
                if (CMP_QNAME(&sp->qname, &qname)) {
                    return slotNum;
                }
            }
        } else {
            for (slotNum = hash->buckets[index]; slotNum >= 0; slotNum = sp->hashChain) {
                sp = &slots[slotNum];
                if (CMP_NAME(&sp->qname, &qname)) {
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
    Validate the supplied slot number. If set to -1, then return the next available property slot number.
    Grow the object if required and update numProp
 */
int ejsGetSlot(Ejs *ejs, EjsPot *obj, int slotNum)
{
    mprAssert(ejsIsPot(ejs, obj));

    if (slotNum < 0) {
        //  MOB - should this be here or only in the VM. probably only in the VM.
        //  MOB -- or move this routine to the VM
        if (!DYNAMIC(obj)) {
            if (obj == ejs->nullValue) {
                ejsThrowReferenceError(ejs, "Object is null");
            } else if (obj == ejs->undefinedValue) {
                ejsThrowReferenceError(ejs, "Object is undefined");
            } else {
                ejsThrowReferenceError(ejs, "Object is not extendable");
            }
            return EJS_ERR;
        }
        slotNum = obj->numProp;
        if (obj->properties == 0 || slotNum >= obj->properties->size) {
            if (growSlots(ejs, obj, obj->numProp + 1) < 0) {
                ejsThrowMemoryError(ejs);
                return EJS_ERR;
            }
        }
        obj->numProp++;
        
    } else if (slotNum >= obj->numProp) {
        if (obj->properties == 0 || slotNum >= obj->properties->size) {
            if (growSlots(ejs, obj, slotNum + 1) < 0) {
                ejsThrowMemoryError(ejs);
                return EJS_ERR;
            }
        }
        obj->numProp = slotNum + 1;
    }
    mprAssert(obj->numProp <= obj->properties->size);
#if BLD_DEBUG
    if (obj == ejs->global && obj->numProp > 220) {
        mprAssert(obj != ejs->global || obj->numProp < 220);
        mprBreakpoint();
    }
#endif
    return slotNum;
}


/**
    Set the value of a property.
    @param slot If slot is -1, then allocate the next free slot
    @return Return the property slot if successful. Return < 0 otherwise.
 */
static int setPotProperty(Ejs *ejs, EjsPot *obj, int slotNum, EjsObj *value)
{
    mprAssert(ejs);
    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(value);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    mprAssert(slotNum < obj->numProp);
    mprAssert(obj->numProp <= obj->properties->size);
    obj->properties->slots[slotNum].value.ref = value;
    return slotNum;
}


/*
    Set the name for a property. Objects maintain a hash lookup for property names. This is hash is created on demand 
    if there are more than N properties. If an object is not dynamic, it will use the types name hash. If dynamic, 
    then the types name hash will be copied when required. 
 */
static int setPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum, EjsName qname)
{
    EjsProperties   *props;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(qname.name);
    mprAssert(qname.space);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    mprAssert(slotNum < obj->numProp);
    props = obj->properties;

    /* Remove the old hash entry if the name will change */
    if (props->slots[slotNum].hashChain >= 0) {
        if (CMP_QNAME(&props->slots[slotNum].qname, &qname)) {
            return slotNum;
        }
        removeHashEntry(ejs, obj, props->slots[slotNum].qname);
    }
    props->slots[slotNum].qname = qname;
    
    mprAssert(slotNum < obj->numProp);
    mprAssert(obj->numProp <= props->size);
    
    if (props->hash || obj->numProp > EJS_HASH_MIN_PROP) {
        if (hashProperty(ejs, obj, slotNum, qname) < 0) {
            ejsThrowMemoryError(ejs);
            return EJS_ERR;
        }
    }
    return slotNum;
}


#if UNUSED
/*
    Set the property Trait. Grow traits if required.
 */
static int setPotPropertyTrait(Ejs *ejs, EjsPot *obj, int slotNum, EjsType *type, int attributes)
{
    EjsSlots   *slots;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= 0);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    slots = obj->properties->slots;
    slots[slotNum].trait.type = type;
    slots[slotNum].trait.attributes = attributes;
    return slotNum;
}
#endif


/******************************* Slot Routines ********************************/
/*
    Grow and object and update numProp and numTraits if required
 */
int ejsGrowPot(Ejs *ejs, EjsPot *obj, int numProp)
{
    mprAssert(ejsIsPot(ejs, obj));

    if (obj->properties == 0 || numProp > obj->properties->size) {
        if (growSlots(ejs, obj, numProp) < 0) {
            return EJS_ERR;
        }
    }
    if (numProp > obj->numProp) {
        obj->numProp = numProp;
    }
    return 0;
}


//  MOB -- inconsistent with growObject which takes numProp. This takes incr.
/*
    Grow the slots, traits, and names by the specified "incr". The new slots|traits|names are created at the "offset"
    Does not update numProp or numTraits.
 */
int ejsInsertPotProperties(Ejs *ejs, EjsPot *obj, int incr, int offset)
{
    EjsSlot         *sp, *slots;
    int             i, size, mark;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(incr >= 0);

    if (incr <= 0) {
        return 0;
    }
    size = obj->numProp + incr;
    if (obj->properties == 0 || obj->properties->size < size) {
        if (growSlots(ejs, obj, size) < 0) {
            return EJS_ERR;
        }
    }
    obj->numProp += incr;
    mprAssert(obj->numProp <= obj->properties->size);
    slots = obj->properties->slots;
    for (mark = offset + incr, i = obj->numProp - 1; i >= mark; i--) {
        sp = &slots[i - mark];
        slots[i] = *sp;
    }
    ejsZeroSlots(ejs, &slots[offset], incr);
    if (ejsMakeHash(ejs, obj) < 0) {
        return EJS_ERR;
    }   
    return 0;
}


/*
    Allocate or grow the slots storage for an object. Does not update numProp.
 */
static int growSlots(Ejs *ejs, EjsPot *obj, int slotCount)
{
    EjsProperties   *props;
    ssize          size;
    int             factor, oldSize;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotCount > 0);
    mprAssert(obj->properties == 0 || slotCount > obj->properties->size);

    props = obj->properties;
    oldSize = props ? props->size : 0;
    
    if (obj == ejs->global) {
        if (slotCount > 500) { //MOB
            mprGetAppDir();
        }
    }

    if (slotCount > oldSize) {
        if (slotCount > EJS_LOTSA_PROP) {
            factor = max(oldSize / 4, EJS_ROUND_PROP);
            slotCount = (slotCount + factor) / factor * factor;
        }
        slotCount = EJS_PROP_ROUNDUP(slotCount);
        size = sizeof(EjsProperties) + (sizeof(EjsSlot) * slotCount);

        if (props == 0) {
            mprAssert(obj->numProp == 0);
            mprAssert(slotCount > 0);
            if ((props = mprAlloc(size)) == 0) {
                return EJS_ERR;
            }
            obj->properties = props;
            ejsZeroSlots(ejs, props->slots, slotCount);
            obj->separateSlots = 1;
        } else {
            if (obj->separateSlots) {
                mprAssert(props->size > 0);
                props = mprRealloc(props, size);
            } else {
                if ((props = mprAlloc(size)) != 0) {
                    memcpy(props, obj->properties, sizeof(EjsProperties) + obj->properties->size * sizeof(EjsSlot));
                    obj->properties = props;
                    obj->separateSlots = 1;
                }
            }
            if (props == 0) {
                return EJS_ERR;
            }
            ejsZeroSlots(ejs, &props->slots[props->size], (slotCount - props->size));
            obj->properties = props;
        }
        props->size = slotCount;
    }
    mprAssert(obj->numProp <= props->size);
    return 0;
}


/*
    Remove a slot and name. Copy up all other properties. WARNING: this can only be used before property binding and 
    should only be used by the compiler.
 */
static void removeSlot(Ejs *ejs, EjsPot *obj, int slotNum, int compact)
{
    EjsSlot     *slots;
    int         i;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= 0);
    mprAssert(compact);

    if (obj->properties) {
        slots = obj->properties->slots;
        if (compact) {
            for (i = slotNum + 1; i < obj->numProp; i++) {
                slots[i - 1] = slots[i];
            }
            obj->numProp--;
            i--;
        } else {
            i = slotNum;
        }
        ejsZeroSlots(ejs, &slots[i], 1);
        ejsMakeHash(ejs, obj);
    }
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
            //  MOB -- why set names to this. Better to set to null?
            sp->qname.name = ejs->emptyString;
            sp->qname.space = ejs->emptyString;
            sp->trait.type = 0;
            sp->trait.attributes = 0;
        }
    }
}


//  MOB - bad API. Should take two EjsPots
void ejsCopySlots(Ejs *ejs, EjsPot *obj, EjsSlot *dest, EjsSlot *src, int count)
{
    while (count-- > 0) {
        *dest = *src;
        dest->hashChain = -1;
        dest++;
        src++;
    }
}


/*
    Remove a property and copy up all other properties. WARNING: This does much more than just a delete and should 
    only be used by the compiler.
 */
int ejsRemovePotProperty(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsPot      *obj;

    mprAssert(ejsIsPot(ejs, vp));

    if (!ejsIsPot(ejs, vp)) {
        ejsThrowTypeError(ejs, "Object is not configurable");
        return EJS_ERR;
    }
    obj = vp;
    mprAssert(ejs);
    mprAssert(obj);
    if (slotNum < 0 || slotNum >= obj->numProp) {
        return EJS_ERR;
    }
    removeSlot(ejs, obj, slotNum, 1);
    return 0;
}


/*********************************** Traits ***********************************/
#if UNUSED

void ejsSetTraitType(Ejs *ejs, EjsTrait *trait, EjsType *type)
{
    mprAssert(trait);
    mprAssert(type == 0 || ejsIsType(ejs, type));
    trait->type = type;
}


void ejsSetTraitAttributes(Ejs *ejs, EjsTrait *trait, int attributes)
{
    mprAssert(trait);
    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);
    trait->attributes = attributes;
}
#endif


static EjsTrait *getPotPropertyTraits(Ejs *ejs, EjsPot *obj, int slotNum)
{
    mprAssert(ejsIsPot(ejs, obj));
    if (slotNum < 0 || slotNum >= obj->numProp) {
        return NULL;
    }
    return &obj->properties->slots[slotNum].trait;
}


static int setPotPropertyTraits(Ejs *ejs, EjsPot *obj, int slotNum, EjsType *type, int attributes)
{
    mprAssert(ejsIsPot(ejs, obj));
    mprAssert(slotNum >= 0);

    if ((slotNum = ejsGetSlot(ejs, obj, slotNum)) < 0) {
        return EJS_ERR;
    }
    if (type) {
        obj->properties->slots[slotNum].trait.type = type;
    }
    if (attributes != -1) {
        obj->properties->slots[slotNum].trait.attributes = attributes;
    }
    return slotNum;
}


#if UNUSED
int ejsGetTraitAttributes(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsPot      *obj;

    if (!ejsIsPot(ejs, vp)) {
        ejsThrowTypeError(ejs, "Object is not configurable");
        return EJS_ERR;
    }
    obj = vp;
    mprAssert(obj);
    mprAssert(slotNum >= 0);

    if (slotNum < 0 || slotNum >= obj->numProp) {
        mprAssert(0);
        return 0;
    }
    return obj->properties->slots[slotNum].trait.attributes;
}


EjsType *ejsGetTraitType(Ejs *ejs, EjsAny *vp, int slotNum)
{
    EjsPot      *obj;

    mprAssert(ejsIsPot(ejs, obj));

    if (!ejsIsPot(ejs, vp)) {
        ejsThrowTypeError(ejs, "Object is not configurable");
        return NULL;
    }
    obj = vp;
    mprAssert(obj);
    mprAssert(slotNum >= 0);
    if (slotNum < 0 || slotNum >= obj->numProp) {
        mprAssert(0);
        return NULL;
    }
    return obj->properties->slots[slotNum].trait.type;
}
#endif


/******************************* Hash Routines ********************************/
/*
    Exponential primes
 */
static int hashSizes[] = {
     19, 29, 59, 79, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 0
};


int ejsGetHashSize(int numProp)
{
    int     i;

    for (i = 0; hashSizes[i]; i++) {
        if (numProp < hashSizes[i]) {
            return hashSizes[i];
        }
    }
    return hashSizes[i - 1];
}


static int hashProperty(Ejs *ejs, EjsPot *obj, int slotNum, EjsName qname)
{
    EjsProperties   *props;
    EjsHash         *hash;
    EjsName         *slotName;
    EjsSlot         *slots;
    int             chainSlotNum, lastSlot, index;

    mprAssert(ejsIsPot(ejs, obj));

    props = obj->properties;
    if (props == NULL || props->hash == NULL || props->hash->size < obj->numProp) {
        /*  Remake the entire hash */
        return ejsMakeHash(ejs, obj);
    }
    hash = props->hash;
    slots = props->slots;
    index = whash(qname.name->value, qname.name->length) % hash->size;

    /* Scan the collision chain */
    lastSlot = -1;
    chainSlotNum = hash->buckets[index];
    mprAssert(chainSlotNum < obj->numProp);
    mprAssert(chainSlotNum < props->size);

    while (chainSlotNum >= 0) {
        slotName = &slots[chainSlotNum].qname;
        if (CMP_QNAME(slotName, &qname)) {
            return 0;
        }
        mprAssert(lastSlot != chainSlotNum);
        lastSlot = chainSlotNum;
        mprAssert(chainSlotNum != slots[chainSlotNum].hashChain);
        chainSlotNum = slots[chainSlotNum].hashChain;
        mprAssert(0 <= lastSlot && lastSlot < props->size);
    }
    if (lastSlot >= 0) {
        mprAssert(lastSlot < obj->numProp);
        mprAssert(lastSlot != slotNum);
        slots[lastSlot].hashChain = slotNum;

    } else {
        /* Start a new hash chain */
        hash->buckets[index] = slotNum;
    }
    slots[slotNum].hashChain = -2;
    slots[slotNum].qname = qname;
    return 0;
}


/*
    Allocate or grow the properties storage for an object. This routine will also manage the hash index for the object. 
    If numInstanceProp is < 0, then grow the number of properties by an increment. Otherwise, set the number of properties 
    to numInstanceProp. We currently don't allow reductions.
 */
//  MOB -- rename
int ejsMakeHash(Ejs *ejs, EjsPot *obj)
{
    EjsSlot         *sp;
    EjsHash         *oldHash, *hash;
    int             i, newHashSize;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));

    if (obj->numProp <= EJS_HASH_MIN_PROP && obj->properties->hash == 0) {
        /* Too few properties */
        return 0;
    }
    /*
        Reallocate the hash buckets if the hash needs to grow larger
     */
    oldHash = obj->properties->hash;
    newHashSize = ejsGetHashSize(obj->numProp);
    if (oldHash == NULL || oldHash->size < newHashSize) {
        hash = (EjsHash*) mprAlloc(sizeof(EjsHash) + (newHashSize * sizeof(int)));
        if (hash == 0) {
            return EJS_ERR;
        }
        hash->buckets = (int*) (((char*) hash) + sizeof(EjsHash));
        hash->size = newHashSize;
        mprAssert(newHashSize > 0);
        obj->properties->hash = hash;
    }
    hash = obj->properties->hash;
    mprAssert(hash);
    memset(hash->buckets, -1, hash->size * sizeof(int));

    /*
        Clear out hash linkage
     */
    if (oldHash) {
        for (sp = obj->properties->slots, i = 0; i < obj->numProp; i++, sp++) {
            sp->hashChain = -1;
        }
    }
    obj->separateHash = 1;

    /*
        Rehash existing properties
     */
    for (sp = obj->properties->slots, i = 0; i < obj->numProp; i++, sp++) {
        if (sp->qname.name && hashProperty(ejs, obj, i, sp->qname) < 0) {
            return EJS_ERR;
        }
    }
    return 0;
}


//  MOB -- rename
void ejsClearPotHash(EjsPot *obj)
{
    EjsSlot         *sp;
    int             i;

    mprAssert(obj);
    mprAssert(ejsIsPot(ejs, obj));

    if (obj->properties->hash) {
        memset(obj->properties->hash->buckets, -1, obj->properties->hash->size * sizeof(int));
        for (sp = obj->properties->slots, i = 0; i < obj->numProp; i++, sp++) {
            sp->hashChain = -1;
        }
    }
}


static void removeHashEntry(Ejs *ejs, EjsPot *obj, EjsName qname)
{
    EjsSlot     *sp;
    EjsName     *nextName;
    int         index, slotNum, lastSlot, *buckets;

    mprAssert(ejsIsPot(ejs, obj));

    if (obj->properties->hash == 0) {
        /*
            No hash. Just do a linear search
         */
        for (slotNum = 0; slotNum < obj->numProp; slotNum++) {
            sp = &obj->properties->slots[slotNum];
            if (CMP_QNAME(&sp->qname, &qname)) {
                //  MOB -- would null be better
                sp->qname.name = ejs->emptyString;
                sp->qname.space = ejs->emptyString;
                sp->hashChain = -1;
                return;
            }
        }
        mprAssert(0);
        return;
    }
    index = whash(qname.name->value, qname.name->length) % obj->properties->hash->size;
    slotNum = obj->properties->hash->buckets[index];
    lastSlot = -1;
    buckets = obj->properties->hash->buckets;
    while (slotNum >= 0) {
        sp = &obj->properties->slots[slotNum];
        nextName = &sp->qname;
        if (CMP_QNAME(nextName, &qname)) {
            if (lastSlot >= 0) {
                obj->properties->slots[lastSlot].hashChain = obj->properties->slots[slotNum].hashChain;
            } else {
                buckets[index] = obj->properties->slots[slotNum].hashChain;
            }
            //  MOB -- null would be better
            sp->qname.name = ejs->emptyString;
            sp->qname.space = ejs->emptyString;
            sp->hashChain = -1;
            return;
        }
        lastSlot = slotNum;
        slotNum = obj->properties->slots[slotNum].hashChain;
    }
    mprAssert(0);
}


int ejsCompactPot(Ejs *ejs, EjsPot *obj)
{
    EjsSlot     *slots, *src, *dest;
    int         i, removed;

    mprAssert(ejsIsPot(ejs, obj));

    src = dest = slots = obj->properties->slots;
    for (removed = i = 0; i < obj->numProp; i++, src++) {
        if (src->value.ref == 0 || src->value.ref == ejs->undefinedValue || src->value.ref == ejs->nullValue) {
            removed++;
            continue;
        }
        *dest++ = *src;
    }
    obj->numProp -= removed;
    ejsMakeHash(ejs, obj);
    return obj->numProp;
}


/************************************ Factory *********************************/
/*
    Create an object which is an instance of a given type. NOTE: only initialize the Object base class. It is up to the 
    caller to complete the initialization for all other base classes by calling the appropriate constructors. The numProp 
    arg is the number of property slots to pre-allocate. It is typically zero and slots are allocated on-demand. If the 
    type creates dynamic instances, then the property slots are allocated separately and can grow. 
 */
void *ejsCreatePot(Ejs *ejs, EjsType *type, int numProp)
{
    EjsPot      *obj, *prototype;

    mprAssert(type);
    
    prototype = (EjsPot*) type->prototype;
    if (type->hasInstanceVars) {
        numProp = max(numProp, prototype->numProp);
    }
    if (type->dynamicInstance) {
        if ((obj = ejsAlloc(ejs, type, 0)) == 0) {
            return 0;
        }
        if (numProp > 0) {
            growSlots(ejs, obj, numProp);
        }
        SET_DYNAMIC(obj, 1);
    } else {
        if ((obj = ejsAlloc(ejs, type, sizeof(EjsProperties) + numProp * sizeof(EjsSlot))) == 0) {
            return 0;
        }
        if (numProp > 0) {
            obj->properties = (EjsProperties*) &(((char*) obj)[type->instanceSize]);
            obj->properties->size = numProp;
        }
    }
    obj->numProp = numProp;
    if (numProp > 0) {
        if (type->hasInstanceVars) {
            if (prototype->numProp > 0) {
                ejsCopySlots(ejs, obj, obj->properties->slots, prototype->properties->slots, prototype->numProp);
            }
            ejsZeroSlots(ejs, &obj->properties->slots[prototype->numProp], obj->properties->size - prototype->numProp);
            if (numProp > EJS_HASH_MIN_PROP) {
                ejsMakeHash(ejs, obj);
            }
        } else {
            ejsZeroSlots(ejs, obj->properties->slots, obj->properties->size);
        }
    }
    ejsSetMemRef(obj);
    return obj;
}


/*
    Manage the object properties for the garbage collector
 */
void ejsManagePot(void *ptr, int flags)
{
    EjsSlot     *sp;
    EjsPot      *obj;
    int         i, numProp;

    if (ptr) {
        obj = (EjsPot*) ptr;

        if (flags & MPR_MANAGE_MARK) {
            if (obj->separateSlots) {
                mprMark(obj->properties);
            }
            if (obj->separateHash) {
                mprMark(obj->properties->hash);
            }
            /*
                Cache numProp incase the object grows while traversing
                //  MOB - assumes that objects dont shrink
             */
            numProp = obj->numProp;
            for (sp = obj->properties->slots, i = 0; i < numProp; i++, sp++) {
                //  MOB -- should not need this test. But seems to be a race with property creation
                if (sp->qname.name) {
                    mprMark(sp->qname.name);
                    mprMark(sp->qname.space);
                    mprMark(sp->value.ref);
#if MOB && CONSIDER
                    mprMark(sp->trait.type);
#endif
                }
            }
        }
    }
}


void ejsCreatePotHelpers(Ejs *ejs)
{
    EjsHelpers      *helpers;

    ejs->potHelpers = ejs->objHelpers;
    helpers = &ejs->potHelpers;
    helpers->clone                  = (EjsCloneHelper) ejsClonePot;
    helpers->create                 = (EjsCreateHelper) ejsCreatePot;
    helpers->defineProperty         = (EjsDefinePropertyHelper) definePotProperty;
    helpers->deleteProperty         = (EjsDeletePropertyHelper) deletePotProperty;
    helpers->deletePropertyByName   = (EjsDeletePropertyByNameHelper) deletePotPropertyByName;
    helpers->getProperty            = (EjsGetPropertyHelper) getPotProperty;
    helpers->getPropertyCount       = (EjsGetPropertyCountHelper) getPotPropertyCount;
    helpers->getPropertyName        = (EjsGetPropertyNameHelper) ejsGetPotPropertyName;
    helpers->getPropertyTraits      = (EjsGetPropertyTraitsHelper) getPotPropertyTraits;
    helpers->lookupProperty         = (EjsLookupPropertyHelper) ejsLookupPotProperty;
    helpers->setProperty            = (EjsSetPropertyHelper) setPotProperty;
    helpers->setPropertyName        = (EjsSetPropertyNameHelper) setPotPropertyName;
    helpers->setPropertyTraits      = (EjsSetPropertyTraitsHelper) setPotPropertyTraits;
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
