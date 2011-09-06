/**
    ejsObject.c - Object class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static EjsObj   *obj_defineProperty(Ejs *ejs, EjsObj *type, int argc, EjsObj **argv);

/************************************* Code ***********************************/
/*
    function get constructor(): Object
 */
static EjsType *obj_constructor(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsType     *constructor;

    if ((constructor = ejsGetPropertyByName(ejs, obj, EN("constructor"))) != 0) {
        return constructor;
    }
    return TYPE(obj);
}


/*
    function get prototype(): Object

    The prototype method is special. It is declared as static so it is generated in the type slots, but it is
    patched to be an instance method so the value of "this" will be preserved when it is invoked.
 */
static EjsAny *obj_prototype(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsFunction     *fun;
    EjsType         *type;
    EjsPot          *prototype;

    if (ejs->compiling) {
        mprAssert(0);
        prototype = ESV(undefined);
        
    } else if (ejsIsType(ejs, obj)) {
        prototype = ((EjsType*) obj)->prototype;
        
    } else if (ejsIsFunction(ejs, obj)) {
        fun = (EjsFunction*) obj;
        if (fun->archetype) {
            prototype = fun->archetype->prototype;
        
        } else {
            type = ejsCreateArchetype(ejs, fun, NULL);
            prototype = type->prototype;
        }
    } else {
        prototype = ESV(undefined);
    }
    return prototype;
}


/*
    function set prototype(p: Object): Void
 */
static EjsObj *obj_set_prototype(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsPot          *prototype;
    EjsFunction     *fun;

    if (ejs->compiling) {
        mprAssert(0);
        return ESV(undefined);
    }
    prototype = (EjsPot*) argv[0];
    if (ejsIsType(ejs, obj)) {
        ((EjsType*) obj)->prototype = prototype;
    } else {
        if (ejsIsFunction(ejs, obj)) {
            fun = (EjsFunction*) obj;
            if (ejsIsType(ejs, fun->archetype)) {
                fun->archetype->prototype = prototype;
            } else {
                ejsCreateArchetype(ejs, fun, prototype);
            }
        } else {
            /*
                Normal property creation. This "prototype" property is not used internally.
             */
            ejsSetPropertyByName(ejs, obj, EN("prototype"), prototype);
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

    deep = (argc == 1 && argv[0] == ESV(true));
    return ejsClone(ejs, obj, deep);
}


/*
    static function create(proto: Object, props: Object = undefined): Objectd 
 */
static EjsObj *obj_create(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsFunction     *constructor;
    EjsObj          *obj, *properties, *options;
    EjsPot          *prototype;
    EjsType         *type;
    EjsName         qname;
    int             count, slotNum;

    prototype = (EjsPot*) argv[0];
    properties = (argc >= 1) ? argv[1] : 0;

    if (ejsIsType(ejs, prototype)) {
        type = (EjsType*) prototype;
    } else {
        constructor = ejsGetPropertyByName(ejs, prototype, EN("constructor"));
        if (constructor) {
            if (ejsIsType(ejs, constructor)) {
                type = (EjsType*) constructor;
            } else if (ejsIsFunction(ejs, constructor)) {
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
            ejsSetPropertyByName(ejs, prototype, EN("constructor"), type);
        }
    }
    obj = ejsCreateObj(ejs, type, 0);
    if (properties) {
        count = ejsGetLength(ejs, properties);
        for (slotNum = 0; slotNum < count; slotNum++) {
            qname = ejsGetPropertyName(ejs, properties, slotNum);
            options = ejsGetProperty(ejs, properties, slotNum);
            argv[0] = obj;
            argv[1] = (EjsObj*) qname.name;
            argv[2] = options;
            obj_defineProperty(ejs, (EjsObj*) type, 3, argv);
        }
    }
    return obj;
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
    int             attributes, slotNum;

    mprAssert(argc == 3);

    obj = argv[0];
    if (!ejsIsPot(ejs, obj)) {
        ejsThrowTypeError(ejs, "Object is not configurable");
        return NULL;
    }
    qname.name = (EjsString*) argv[1];
    qname.space = ESV(empty);
    options = argv[2];
    value = 0;
    set = get = 0;
    attributes = 0;

    if ((namespace = ejsGetPropertyByName(ejs, options, EN("namespace"))) != 0) {
        qname.space = (EjsString*) namespace;
    }
    if ((slotNum = ejsLookupProperty(ejs, obj, qname)) >= 0) {
        if (ejsPropertyHasTrait(ejs, obj, slotNum, EJS_TRAIT_FIXED)) {
            ejsThrowTypeError(ejs, "Property \"%N\" is not configurable", qname);
            return 0;
        }
    }
    type = ejsGetPropertyByName(ejs, options, EN("type"));

    if ((configurable = ejsGetPropertyByName(ejs, options, EN("configurable"))) != 0) {
        if (configurable == ESV(false)) {
            attributes |= EJS_TRAIT_FIXED;
        }
    }
    if ((enumerable = ejsGetPropertyByName(ejs, options, EN("enumerable"))) != 0) {
        if (enumerable == ESV(false)) {
            attributes |= EJS_TRAIT_HIDDEN;
        }
    }
    value = ejsGetPropertyByName(ejs, options, EN("value"));
    if (value && type && !ejsIsA(ejs, value, type)) {
        ejsThrowArgError(ejs, "Value is not of the required type");
        return 0;
    }
    if ((get = ejsGetPropertyByName(ejs, options, EN("get"))) != 0) {
        if (ejsIsFunction(ejs, get)) {
            EjsName qset = { qname.space, ejsCreateStringFromAsc(ejs, "set") };
            get->setter = ejsGetPropertyByName(ejs, obj, qset);
            attributes |= EJS_TRAIT_GETTER;
        } else {
            ejsThrowArgError(ejs, "The \"get\" property is not a function");
            return 0;
        }
    }
    if ((set = ejsGetPropertyByName(ejs, options, EN("set"))) != 0) {
        if (ejsIsFunction(ejs, set)) {
            if (get == 0 && (fun = ejsGetPropertyByName(ejs, obj, qname)) != 0) {
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
    if ((writable = ejsGetPropertyByName(ejs, options, EN("writable"))) != 0) {
        if (writable == ESV(false)) {
            attributes |= EJS_TRAIT_READONLY;
        }
    }
    mprAssert((attributes & EJS_TRAIT_MASK) == attributes);
    if (ejsDefineProperty(ejs, obj, -1, qname, type, attributes, value) < 0) {
        ejsThrowTypeError(ejs, "Can't define property %@", qname.name);
    }
    return 0;
}


/*
    static function freeze(obj: Object): Void
 */
static EjsObj *obj_freeze(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    EjsTrait    *trait;
    int         slotNum, numProp;

    obj = (EjsObj*) argv[0];
    SET_DYNAMIC(obj, 0);
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
            ejsSetPropertyTraits(ejs, obj, slotNum, NULL, trait->attributes | EJS_TRAIT_READONLY | EJS_TRAIT_FIXED);
        }
    }
    if (ejsIsType(ejs, obj)) {
        obj = (EjsObj*) ((EjsType*) obj)->prototype;
        numProp = ejsGetLength(ejs, obj);
        for (slotNum = 0; slotNum < numProp; slotNum++) {
            if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
                ejsSetPropertyTraits(ejs, obj, slotNum, NULL, trait->attributes | EJS_TRAIT_READONLY | EJS_TRAIT_FIXED);
            }
        }
    }
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
    int         numProp;

    obj = ip->target;
    numProp = ejsGetLength(ejs, obj);
    for (; ip->index < numProp; ip->index++) {
        qname = ejsGetPropertyName(ejs, obj, ip->index);
        if (qname.name == NULL) {
            continue;
        }
        trait = ejsGetPropertyTraits(ejs, obj, ip->index);
        if (trait && trait->attributes & 
                (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
            continue;
        }
        ip->index++;
        return (EjsObj*) qname.name;
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return the default iterator.

    iterator function get(options: Object = null): Iterator
 */
static EjsIterator *obj_get(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, obj, nextObjectKey, 0, NULL);
}


/*
    Function to iterate and return the next element value.
    NOTE: this is not a method of Object. Rather, it is a callback function for Iterator
 */
static EjsObj *nextObjectValue(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    EjsTrait    *trait;
    int         numProp;

    obj = ip->target;
    numProp = ejsGetLength(ejs, obj);
    for (; ip->index < numProp; ip->index++) {
        trait = ejsGetPropertyTraits(ejs, obj, ip->index);
        if (trait && trait->attributes & 
                (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
            continue;
        }
        return ejsGetProperty(ejs, obj, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    Return an iterator to return the next array element value.

    iterator function getValues(options: Object = null): Iterator
 */
static EjsIterator *obj_getValues(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, obj, nextObjectValue, 0, NULL);
}


/*
    Get the number of properties in the object.

    function get getOwnPropertyCount(obj): Number
 */
static EjsNumber *obj_getOwnPropertyCount(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return ejsCreateNumber(ejs, ejsGetLength(ejs, obj) - TYPE(obj)->numInherited);
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
    int             slotNum;

    obj = argv[0];
    //  TODO - ugly
    qname.space = ESV(empty);
    qname.name = (EjsString*) argv[1];
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, qname, &lookup)) < 0) {
        return ESV(false);
    }
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    result = ejsCreateEmptyPot(ejs);
    value = ejsGetVarByName(ejs, obj, qname, &lookup);
    if (value == 0) {
        value = ESV(null);
    }
    type = (trait) ? trait->type: 0;
    if (trait && trait->attributes & EJS_TRAIT_GETTER) {
        ejsSetPropertyByName(ejs, result, EN("get"), value);
    } else if (trait && trait->attributes & EJS_TRAIT_SETTER) {
        fun = (EjsFunction*) value;
        if (ejsIsFunction(ejs, fun)) {
            ejsSetPropertyByName(ejs, result, EN("set"), fun->setter);
        }
    } else if (value) {
        ejsSetPropertyByName(ejs, result, EN("value"), value);
    }
    ejsSetPropertyByName(ejs, result, EN("configurable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_FIXED)));
    ejsSetPropertyByName(ejs, result, EN("enumerable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_HIDDEN)));
    qn = ejsGetPropertyName(ejs, obj, slotNum);
    ejsSetPropertyByName(ejs, result, EN("namespace"), qn.space);
    ejsSetPropertyByName(ejs, result, EN("type"), type ? (EjsObj*) type : ESV(null));
    ejsSetPropertyByName(ejs, result, EN("writable"), 
        ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_READONLY)));
    return result;
}


/*
    Get all properties names including non-enumerable properties

    static function getOwnPropertyNames(obj: Object, options: Object): Array
 */
static EjsArray *obj_getOwnPropertyNames(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj, *options, *arg;
    EjsArray    *result;
    EjsTrait    *trait;
    EjsName     qname;
    int         slotNum, numProp, index, includeBases, excludeFunctions;

    obj = (EjsObj*) argv[0];
    includeBases = 0;
    excludeFunctions = 0;
    if (argc > 0) {
        options = argv[1];
        if ((arg = ejsGetPropertyByName(ejs, options, EN("includeBases"))) != 0) {
            includeBases = (arg == ESV(true));
        }
        if ((arg = ejsGetPropertyByName(ejs, options, EN("excludeFunctions"))) != 0) {
            excludeFunctions = (arg == ESV(true));
        }
    }
    if ((result = ejsCreateArray(ejs, 0)) == 0) {
        return 0;
    }
    index = 0;
    slotNum = (includeBases) ? 0 : TYPE(obj)->numInherited;
    numProp = ejsGetLength(ejs, obj);
    for (; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
            if (trait->attributes & (EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
                continue;
            }
        }
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        if (excludeFunctions && ejsIsFunction(ejs, ejsGetProperty(ejs, obj, slotNum))) {
            continue;
        }
        ejsSetProperty(ejs, result, index++, qname.name);
    }
    if (ejsIsType(ejs, obj) || ejsIsFunction(ejs, obj)) {
        if (ejsLookupProperty(ejs, obj, EN("prototype")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateStringFromAsc(ejs, "prototype"));
        }
        if (ejsLookupProperty(ejs, obj, EN("length")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateStringFromAsc(ejs, "length"));
        }
    } else if (ejsIsPrototype(ejs, obj)) {
        if (ejsLookupProperty(ejs, obj, EN("constructor")) < 0) {
            ejsSetProperty(ejs, result, index++, ejsCreateStringFromAsc(ejs, "constructor"));
        }
    }
    return result;
}


/*
    static function getOwnPrototypeOf(obj: Object): Object
 */
static EjsPot *obj_getOwnPrototypeOf(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return TYPE(argv[0])->prototype;
}


/*
    function hasOwnProperty(name: String): Boolean
 */
static EjsBoolean *obj_hasOwnProperty(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsName     qname;
    EjsLookup   lookup;
    int         slotNum;

    qname.space = ESV(empty);
    qname.name = (EjsString*) argv[0];
    slotNum = ejsLookupVarWithNamespaces(ejs, obj, qname, &lookup);
    return ejsCreateBoolean(ejs, slotNum >= 0);
}


/*
    static function isExtensible(obj: Object): Boolean
 */
static EjsBoolean *obj_isExtensible(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return ejsCreateBoolean(ejs, DYNAMIC(obj));
}


/*
    static function isFrozen(obj: Object): Boolean
 */
static EjsBoolean *obj_isFrozen(Ejs *ejs, EjsObj *type, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsPot      *obj;
    int         numProp, frozen, slotNum;

    obj = (EjsPot*) argv[0];
    frozen = 1;
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
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
    if (DYNAMIC(obj)) {
        frozen = 0;
    }
    return ejsCreateBoolean(ejs, frozen);
}


/*
    static function isPrototypeOf(obj: Object): Boolean
 */
static EjsBoolean *obj_isPrototypeOf(Ejs *ejs, EjsObj *prototype, int argc, EjsObj **argv)
{
    EjsObj  *obj;
    
    obj = argv[0];
    return (prototype == ((EjsObj*) TYPE(obj)->prototype)) ? ESV(true) : ESV(false);
}


/*
    static function isSealed(obj: Object): Boolean
 */
static EjsBoolean *obj_isSealed(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsPot      *obj;
    int         numProp, sealed, slotNum;

    obj = (EjsPot*) argv[0];
    sealed = 1;
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
            if (!(trait->attributes & EJS_TRAIT_FIXED)) {
                sealed = 0;
                break;
            }
        }
    }
    if (DYNAMIC(obj)) {
        sealed = 0;
    }
    return ejsCreateBoolean(ejs, sealed);
}


#if FUTURE
/*
    Get enumerable properties names

    static function keys(obj: Object): Array
 */
static EjsArray *obj_keys(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj, *vp;
    EjsArray    *result;
    EjsTrait    *trait;
    EjsName     qname;
    int         numProp, slotNum;

    obj = argv[0];
    numProp = ejsGetLength(ejs, obj);
    if ((result = ejsCreateArray(ejs, numProp)) == 0) {
        return 0;
    }
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
            if (trait->attributes & EJS_TRAIT_DELETED) {
                continue;
            }
        }
        vp = ejsGetProperty(ejs, obj, slotNum);
        qname = ejsGetPropertyName(ejs, obj, slotNum);
        ejsSetProperty(ejs, result, slotNum, ejsCreateStringFromAsc(ejs, qname.name));
    }
    return result;
}
#endif


/*
    static function preventExtensions(obj: Object): Object
 */
static EjsObj *obj_preventExtensions(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    SET_DYNAMIC(obj, 0);
    return obj;
}


/*
    static function seal(obj: Object): Void
 */
static EjsObj *obj_seal(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;
    EjsTrait    *trait;
    int         slotNum, numProp;

    obj = argv[0];
    numProp = ejsGetLength(ejs, obj);
    for (slotNum = 0; slotNum < numProp; slotNum++) {
        if ((trait = ejsGetPropertyTraits(ejs, obj, slotNum)) != 0) {
            trait->attributes |= EJS_TRAIT_FIXED;
        }
    }
    SET_DYNAMIC(obj, 0);
    return 0;
}


static int sortSlots(cvoid *a1, cvoid *a2)
{
    EjsSlot *s1, *s2;

    s1 = (EjsSlot*) a1;
    s2 = (EjsSlot*) a2;
    if (s1->qname.name->value == s2->qname.name->value) {
        return 0;
    } else if (scmp(s1->qname.name->value, s2->qname.name->value) < 0) {
        return -1;
    } 
    return 1;
}


/*
    static function sortProperties(obj: Object, ascending: Boolean = true): Void
 */
static EjsObj *obj_sortProperties(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsPot      *obj;
    int         asc;

    obj = (EjsPot*) argv[0];
    asc = (argc >= 2 && argv[1] == ESV(true));
    qsort(obj->properties->slots, obj->numProp, sizeof(EjsSlot), sortSlots);
    ejsIndexProperties(ejs, obj);
    return 0;
}


/*
    function propertyIsEnumerable(property: String, flag: Object = undefined): Boolean
 */
static EjsBoolean *obj_propertyIsEnumerable(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    EjsTrait    *trait;
    EjsName     qname;
    EjsLookup   lookup;
    int         slotNum;

    mprAssert(argc == 1 || argc == 2);

    qname.space = ESV(empty);
    qname.name = (EjsString*) argv[0];
    if ((slotNum = ejsLookupVarWithNamespaces(ejs, obj, qname, &lookup)) < 0) {
        return ESV(false);
    }
    trait = ejsGetPropertyTraits(ejs, obj, slotNum);
    return ejsCreateBoolean(ejs, !trait || !(trait->attributes & EJS_TRAIT_HIDDEN));
}


/*
    Convert the object to a JSON string. This also handles Json for Arrays.

    function toJSON(options: Object = null): String
 */
EjsString *ejsObjToJSON(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return ejsSerializeWithOptions(ejs, vp, (argc == 1) ? argv[0] : NULL);
}


#if ES_Object_toLocaleString
/*
    Convert the object to a localized string

    function toLocaleString(): String
 */
static EjsString *toLocaleString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return ejsObjToString(ejs, vp, argc, argv);
}
#endif


EjsString *ejsObjToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    if (ejsIs(ejs, vp, String)) {
        return (EjsString*) vp;
    }
    return (ejs->service->objHelpers.cast)(ejs, vp, ESV(String));
}


/************************************************** Reflection **********************************************/
/*
    Get the base class of a type object.

    function getBaseType(obj: Type): Type
 */
static EjsType *obj_getBaseType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *vp;

    vp = argv[0];
    if (ejsIsType(ejs, vp)) {
        return (((EjsType*) vp)->baseType);
    }
    return ESV(null);
}


/*
    function isPrototype(obj: Object): Boolean
 */
static EjsBoolean *obj_isPrototype(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, ejsIsPrototype(ejs, argv[0]));
}


/*
    function isType(obj: Object): Boolean
 */
static EjsBoolean *obj_isType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, ejsIsType(ejs, argv[0]));
}


/*
    Get the type of the object.

    function getType(obj: Object): Type
 */
static EjsType *obj_getType(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];
    return TYPE(obj);
}


/*
    Return the name of the type of an object. If the obj is a type, get the base type.
 */
EjsString *ejsGetTypeName(Ejs *ejs, EjsAny *obj)
{
    EjsType     *type;

    if (obj == 0) {
        return ESV(undefined);
    }
    type = (EjsType*) TYPE(obj);
    if (type == 0) {
        return ESV(null);
    }
    return type->qname.name;
}


/*
    function getTypeName(obj): String
 */
static EjsString *obj_getTypeName(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the name of a function or type object

    function getName(obj: Object): String
 */
static EjsString *obj_getName(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    obj = argv[0];

    if (ejsIsType(ejs, obj)) {
        return ((EjsType*) obj)->qname.name;
    } else if (ejsIsFunction(ejs, obj)) {
        return ((EjsFunction*) obj)->name;
    }
    return ESV(empty);
}

/*********************************** Globals **********************************/
/*
    function typeOf(obj): String
 */
static EjsString *obj_typeOf(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv)
{
    mprAssert(argc >= 1);
    return ejsGetTypeName(ejs, argv[0]);
}


/*
    Get the ecma "typeof" value for an object. Unfortunately, typeof is pretty lame.
 */
EjsString *ejsGetTypeOf(Ejs *ejs, EjsAny *vp)
{
    cchar   *word;

    if (vp == ESV(undefined)) {
        word = "undefined";

    } else if (ejsIs(ejs, vp, Null)) {
        /* Yea - I know, ECMAScript is broken */
        word = "object";

    } else if (ejsIs(ejs, vp, Boolean)) {
        word = "boolean";

    } else if (ejsIs(ejs, vp, Number)) {
        word = "number";

    } else if (ejsIs(ejs, vp, String)) {
        word = "string";

    } else if (ejsIs(ejs, vp, Function)) {
        word = "function";
               
    } else {
        word = "object";
    }
    return ejsCreateStringFromAsc(ejs, word);
}


void ejsConfigureObjectType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;
    EjsFunction *fun;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Object"))) != 0) {
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
        ejsBindAccess(ejs, type, ES_Object_prototype, obj_prototype, obj_set_prototype);
        ejsBindMethod(ejs, type, ES_Object_seal, obj_seal);
#if ES_Object_sortProperties
        ejsBindMethod(ejs, type, ES_Object_sortProperties, obj_sortProperties);
#endif

        /* Reflection */
        ejsBindMethod(ejs, type, ES_Object_getBaseType, obj_getBaseType);
        ejsBindMethod(ejs, type, ES_Object_getType, obj_getType);
        ejsBindMethod(ejs, type, ES_Object_getTypeName, obj_getTypeName);
        ejsBindMethod(ejs, type, ES_Object_getName, obj_getName);
        ejsBindMethod(ejs, type, ES_Object_isPrototype, obj_isPrototype);
        ejsBindMethod(ejs, type, ES_Object_isType, obj_isType);

        ejsBindMethod(ejs, prototype, ES_Object_constructor, obj_constructor);
        ejsBindMethod(ejs, prototype, ES_Object_clone, obj_clone);
        ejsBindMethod(ejs, prototype, ES_Object_iterator_get, obj_get);
        ejsBindMethod(ejs, prototype, ES_Object_iterator_getValues, obj_getValues);
        ejsBindMethod(ejs, prototype, ES_Object_hasOwnProperty, obj_hasOwnProperty);
        ejsBindMethod(ejs, prototype, ES_Object_isPrototypeOf, obj_isPrototypeOf);
        ejsBindMethod(ejs, prototype, ES_Object_propertyIsEnumerable, obj_propertyIsEnumerable);
        ejsBindMethod(ejs, prototype, ES_Object_toLocaleString, toLocaleString);
        ejsBindMethod(ejs, prototype, ES_Object_toString, ejsObjToString);
        ejsBindMethod(ejs, prototype, ES_Object_toJSON, ejsObjToJSON);
        /*
            The prototype method is special. It is declared as static so it is generated in the type slots, but it is
            patched to be an instance method so the value of "this" will be preserved when it is invoked.
         */
        fun = ejsGetProperty(ejs, type, ES_Object_prototype);
        fun->staticMethod = 0;
        fun->setter->staticMethod = 0;
        type->constructor.block.pot.properties->slots[ES_Object_prototype].trait.attributes &= ~EJS_PROP_STATIC;
    }
    ejsBindFunction(ejs, ejs->global, ES_typeOf, obj_typeOf);

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
