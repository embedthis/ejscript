/**
    ejsHelper.c - Helper methods for types.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static MprNumber parseNumber(Ejs *ejs, cchar *str);
static bool      parseBoolean(Ejs *ejs, cchar *s);

/************************************* Code ***********************************/
/**
    Cast the variable to a given target type.
    @return Returns a variable with the result of the cast or null if an exception is thrown.
 */
EV *ejsCast(Ejs *ejs, EV *ev, EjsType *targetType)
{
    EjsType     *type;
    EV          *result;
    int         save;

    mprAssert(ejs);
    mprAssert(type);
    mprAssert(ev);

    if (ev == 0) {
        //  MOB -- required?
        ev = (EV*) ejs->undefinedValue;
    }
    type = TYPE(ev);
    if (type == targetType) {
        return ev;
    }
    if (type->helpers.cast) {
        /*
            Don't GC cast operations. This enables native code to not worry about the GC collecting tmp objects
            MOB - what other operations should this apply to?
         */
        save = ejs->heap->enabled;
        ejs->heap->enabled = 0;
        result = (type->helpers.cast)(ejs, ev, targetType);
        ejs->heap->enabled = save;
        return result;
    }
    ejsThrowInternalError(ejs, "Cast helper not defined for type \"%s\"", type->qname.name);
    return 0;
}


/*
    Create a new instance of a variable. Delegate to the type specific create.
 */
EV *ejsCreate(Ejs *ejs, EjsType *type, int numSlots)
{
#if VXWORKS
    /*
     *  The VxWorks cc386 invoked linker crashes without this test. Ugh!
     */
    if (type == 0) {
        return 0;
    }
#endif
    mprAssert(type->helpers.create);
    return (type->helpers.create)(ejs, type, numSlots);
}


/**
    Copy a variable by copying all properties. If a property is a reference  type, just copy the reference.
    See ejsDeepClone for a complete recursive copy of all reference contents.
    @return Returns a variable or null if an exception is thrown.
 */
EV *ejsClone(Ejs *ejs, EV *ev, bool deep)
{
    EV  *result;
    
    if (ev == 0) {
        return 0;
    }
    mprAssert(TYPE(ev)->helpers.clone);
    if (VISITED(ev) == 0) {
        VISITED(ev) = 1;
        result = (TYPE(ev)->helpers.clone)(ejs, ev, deep);
        VISITED(ev) = 0;
    } else {
        result = ev;
    }
    return result;
}


/*
    Define a property and its traits.
    @return Return the slot number allocated for the property.
 */
int ejsDefineProperty(Ejs *ejs, EV *ev, int slotNum, EjsName name, EjsType *propType, int64 attributes, EV *value)
{
    mprAssert(name.name);
    mprAssert(name.space);
    
    return (TYPE(ev)->helpers.defineProperty)(ejs, ev, slotNum, name, propType, attributes, value);
}


/**
    Delete a property in a value
    @return Returns a status code.
 */
int ejsDeleteProperty(Ejs *ejs, EV *ev, int slotNum)
{
    EjsType     *type;

    mprAssert(slotNum >= 0);
    
    type = TYPE(ev);
    mprAssert(type->helpers.deleteProperty);
    return (type->helpers.deleteProperty)(ejs, ev, slotNum);
}


/**
    Delete a property
    @return Returns a status code.
 */
int ejsDeletePropertyByName(Ejs *ejs, EV *ev, EjsName qname)
{
    EjsLookup   lookup;
    int         slotNum;

    mprAssert(qname.name);
    mprAssert(qname.space);
    
    if (TYPE(ev)->helpers.deletePropertyByName) {
        return (TYPE(ev)->helpers.deletePropertyByName)(ejs, ev, qname);
    } else {
        slotNum = ejsLookupVar(ejs, ev, qname, &lookup);
        if (slotNum < 0) {
            ejsThrowReferenceError(ejs, "Property \"%S\" does not exist", qname.name);
            return 0;
        }
        return ejsDeleteProperty(ejs, ev, slotNum);
    }
}


#if UNUSED
void ejsDestroy(Ejs *ejs, EV *ev)
{
    EjsType     *type;

    mprAssert(ev);

    type = TYPE(ev);
    mprAssert(type->helpers.destroy);
    (type->helpers.destroy)(ejs, ev);
}
#endif

/**
    Get a property at a given slot in a variable.
    @return Returns the requested property varaible.
 */
void *ejsGetProperty(Ejs *ejs, EV *ev, int slotNum)
{
    EjsType     *type;

    mprAssert(ejs);
    mprAssert(ev);

    type = TYPE(ev);
    mprAssert(type->helpers.getProperty);
    return (type->helpers.getProperty)(ejs, ev, slotNum);
}


void *ejsGetPropertyByName(Ejs *ejs, EV *ev, EjsName name)
{
    EjsType     *type;
    int         slotNum;

    mprAssert(ejs);
    mprAssert(ev);

    type = TYPE(ev);

    /*
        WARNING: this is not implemented by most types
     */
    if (type->helpers.getPropertyByName) {
        return (type->helpers.getPropertyByName)(ejs, ev, name);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, ev, name);
    if (slotNum < 0) {
        return 0;
    }
    return ejsGetProperty(ejs, ev, slotNum);
}


/**
    Return the number of properties in the variable.
    @return Returns the number of properties.
 */
int ejsGetLength(Ejs *ejs, EV *ev)
{
    mprAssert(TYPE(ev)->helpers.getPropertyCount);
    return (TYPE(ev)->helpers.getPropertyCount)(ejs, ev);
}


/**
    Return the name of a property indexed by slotNum.
    @return Returns the property name.
 */
EjsName ejsGetPropertyName(Ejs *ejs, EV *ev, int slotNum)
{
    EjsType     *type;

    type = TYPE(ev);
    mprAssert(type->helpers.getPropertyName);
    return (type->helpers.getPropertyName)(ejs, ev, slotNum);
}


/**
    Get a property slot. Lookup a property name and return the slot reference. If a namespace is supplied, the property
    must be defined with the same namespace.
    @return Returns the slot number or -1 if it does not exist.
 */
int ejsLookupProperty(Ejs *ejs, EV *ev, EjsName name)
{
    mprAssert(ejs);
    mprAssert(ev);
    mprAssert(name.name);

    mprAssert(TYPE(ev)->helpers.lookupProperty);
    return (TYPE(ev)->helpers.lookupProperty)(ejs, ev, name);
}


/*
    Invoke an operator.
    ev is left-hand-side
    @return Return a variable with the result or null if an exception is thrown.
 */
EV *ejsInvokeOperator(Ejs *ejs, EV *ev, int opCode, EV *rhs)
{
    mprAssert(ev);

    mprAssert(TYPE(ev)->helpers.invokeOperator);
    return (TYPE(ev)->helpers.invokeOperator)(ejs, ev, opCode, rhs);
}


/*
    ejsMark is in ejsGarbage.c
 */


/*
    Set a property and return the slot number. Incoming slot may be -1 to allocate a new slot.
 */
int ejsSetProperty(Ejs *ejs, EV *ev, int slotNum, EV *value)
{
    mprAssert(ev);

    if (ev == 0) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    mprAssert(TYPE(ev)->helpers.setProperty);
    return (TYPE(ev)->helpers.setProperty)(ejs, ev, slotNum, value);
}


/*
    Set a property given a name.
 */
int ejsSetPropertyByName(Ejs *ejs, EV *ev, EjsName qname, EV *value)
{
    int     slotNum;

    mprAssert(ejs);
    mprAssert(ev);

    /*
        WARNING: Not all types implement this
     */
    if (TYPE(ev)->helpers.setPropertyByName) {
        return (TYPE(ev)->helpers.setPropertyByName)(ejs, ev, qname, value);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, ev, qname);
    if (slotNum < 0) {
        slotNum = ejsSetProperty(ejs, ev, -1, value);
        if (slotNum < 0) {
            return EJS_ERR;
        }
        if (ejsSetPropertyName(ejs, ev, slotNum, qname) < 0) {
            return EJS_ERR;
        }
        return slotNum;
    }
    return ejsSetProperty(ejs, ev, slotNum, value);
}


/*
    Set the property name and return the slot number. Slot may be -1 to allocate a new slot.
 */
int ejsSetPropertyName(Ejs *ejs, EV *ev, int slot, EjsName qname)
{
    mprAssert(TYPE(ev)->helpers.setPropertyName);
    return (TYPE(ev)->helpers.setPropertyName)(ejs, ev, slot, qname);
}


/**
    Get a string representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToString(Ejs *ejs, EV *ev)
{
    if (ev == 0 || ejsIsString(ejs, ev)) {
        return (EjsString*) ev;
    }
    return (EjsString*) ejsCast(ejs, ev, ejs->stringType);
}


/**
    Get a numeric representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsNumber *ejsToNumber(Ejs *ejs, EV *ev)
{
    if (ev == 0 || ejsIsNumber(ejs, ev)) {
        return (EjsNumber*) ev;
    }
    if (TYPE(ev)->helpers.cast) {
        return (EjsNumber*) (TYPE(ev)->helpers.cast)(ejs, ev, ejs->numberType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", TYPE(ev)->qname.name);
    return 0;
}


/**
    Get a boolean representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsBoolean *ejsToBoolean(Ejs *ejs, EV *ev)
{
    if (ev == 0 || ejsIsBoolean(ejs, ev)) {
        return (EjsBoolean*) ev;
    }
    if (TYPE(ev)->helpers.cast) {
        return (EjsBoolean*) (TYPE(ev)->helpers.cast)(ejs, ev, ejs->booleanType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", TYPE(ev)->qname.name);
    return 0;
}


/**
    Get a serialized string representation of a variable using JSON encoding.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToJSON(Ejs *ejs, EV *ev, EV *options)
{
    EjsFunction     *fn;
    EjsString       *result;
    EV              *argv[1];
    EjsName         qname;
    int             argc;

    if (ev == 0) {
        return ejsCreateStringFromCS(ejs, "undefined");
    }
    fn = (EjsFunction*) ejsGetPropertyByName(ejs, (EV*) TYPE(ev)->prototype, N(NULL, "toJSON"));
    if (fn == 0) {
        fn = (EjsFunction*) ejsGetPropertyByName(ejs, (EV*) ejs->objectType->prototype, qname);        
    }
    if (ejsIsFunction(ejs, fn)) {
        if (options) {
            argc = 1;
            argv[0] = options;
        } else {
            argc = 0;
            argv[0] = NULL;
        }
        result = (EjsString*) ejsRunFunction(ejs, fn, ev, argc, argv);
    } else {
        result = ejsToString(ejs, ev);
        result = ejsToJSON(ejs, (EV*) result, options);
    }
    return result;
}


/**
    Get a Path representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsPath *ejsToPath(Ejs *ejs, EV *ev)
{
    if (ev == 0 || ejsIsPath(ejs, ev)) {
        return (EjsPath*) ev;
    }
    return (EjsPath*) ejsCast(ejs, ev, ejs->pathType);
}


/**
    Get a Uri representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsUri *ejsToUri(Ejs *ejs, EV *ev)
{
    if (ev == 0 || ejsIsUri(ejs, ev)) {
        return (EjsUri*) ev;
    }
    return (EjsUri*) ejsCast(ejs, ev, ejs->uriType);
}


/*
    Fully construct a new object. We create a new instance and call all required constructors.
 */
EV *ejsCreateInstance(Ejs *ejs, EjsType *type, int argc, EV **argv)
{
    EV      *ev;

    mprAssert(type);

    ev = ejsCreate(ejs, type, 0);
    if (ev == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (type->constructor.block.obj.isFunction) {
        ejsMakePermanent(ejs, ev);
        ejsRunFunction(ejs, (EjsFunction*) type, ev, argc, argv);
        ejsMakeTransient(ejs, ev);
    }
    return ev;
}



#if UNUSED
int ejsIs(EV *ev, int slot)
{
    EjsType     *tp;

    if (ev == 0) {
        return 0;
    }
    if (TYPE(ev)->id == slot) {
        return 1;
    }
    for (tp = ((EjsType*) TYPE(ev))->baseType; tp; tp = tp->baseType) {
        if (tp->id == slot) {
            return 1;
        }
    }
    return 0;
}
#endif

#if UNUSED
/************************************* Misc ***********************************/

static void missingHelper(Ejs *ejs, EV *obj, cchar *helper) 
{
    EjsType     *type;

    type = TYPE(obj);
    ejsThrowInternalError(ejs, "The \"%s\" helper is not defined for this type \"%s\"", type->qname.name);
}


static EV *castObject(Ejs *ejs, EV *obj, EjsType *type)
{
    missingHelper(ejs, obj, "cast");
}


static EV *create(Ejs *ejs, EjsType *type, int numSlots)
{
    missingHelper(ejs, obj, "create");
}


void *ejsCloneObject(Ejs *ejs, void *vp, bool deep)
{
    return dest;
}


static int defineObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsName qname, EjsType *propType, int64 attributes, 
    EjsObj *value)
{
}


static int deleteObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    return 0;
}


static int deleteObjectPropertyByName(Ejs *ejs, EjsObj *obj, EjsName qname)
{
}


static EjsObj *getObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
}


static int getObjectPropertyCount(Ejs *ejs, EjsObj *obj)
{
}


EjsName ejsGetObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum)
{
}


int ejsLookupObjectProperty(struct Ejs *ejs, EjsObj *obj, EjsName qname)
{
}


void ejsMarkObject(Ejs *ejs, void *ptr)
{
}


static int setObjectProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsObj *value)
{
}


static int setObjectPropertyName(Ejs *ejs, EjsObj *obj, int slotNum, EjsName qname)
{
}
#endif

/************************************* Misc ***********************************/

#if UNUSED
EjsName *ejsAllocName(MprCtx ctx, cchar *name, cchar *space)
{
    EjsName     *np;

    np = mprAllocObj(ctx, EjsName, NULL);
    if (np) {
        np->name = mprStrdup(np, name);
        np->space = mprStrdup(np, space);
    }
    return np;
}


EjsName ejsCopyName(MprCtx ctx, EjsName qname)
{
    EjsName     name;

    name.name = mprStrdup(ctx, qname->name);
    name.space = mprStrdup(ctx, qname->space);
    return name;
}


EjsName *ejsDupName(MprCtx ctx, EjsName qname)
{
    return ejsAllocName(ctx, qname->name, qname->space);
}
#endif


EjsName ejsName(Ejs *ejs, cchar *space, cchar *name)
{
    EjsName     n;

    n.name = ejsCreateStringFromCS(ejs, name);
    n.space = ejsCreateStringFromCS(ejs, space);
    return n;
}


/*
    Parse a string based on formatting instructions and intelligently create a variable.
    Number formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
        [+|-][DIGITS][.][DIGITS][(e|E)[+|-]DIGITS]
 */
//  MOB -- should this be EjsString?
EV *ejsParse(Ejs *ejs, cchar *str, int preferredType)
{
    EjsChar     *buf;
    int         type;

    mprAssert(str);

    buf = (char*) str;
    type = preferredType;

    //  MOB unicode
    while (isspace((int) *buf)) {
        buf++;
    }    
    if (preferredType == ES_Void || preferredType < 0) {
        if (*buf == '-' || *buf == '+') {
            type = ejs->numberType->id;

        } else if (*buf == '/') {
            type = ES_RegExp;

        } else if (!isdigit((int) *buf) && *buf != '.') {
            if (strcmp(buf, "true") == 0) {
                return ejs->trueValue;

            } else if (strcmp(buf, "false") == 0) {
                return ejs->falseValue;
            }
            type = ES_String;

            if (strcmp(buf, "true") == 0 || strcmp(buf, "false") == 0) {
                type = ES_Boolean;
            } else {
                type = ES_String;
            }

        } else {
            type = ES_Number;
        }
    }
    switch (type) {
    case ES_Object:
    case ES_Void:
    case ES_Null:
    default:
        break;

    case ES_Number:
        return (EV*) ejsCreateNumber(ejs, parseNumber(ejs, buf));

    case ES_Boolean:
        return (EV*) ejsCreateBoolean(ejs, parseBoolean(ejs, buf));

#if BLD_FEATURE_REGEXP
    case ES_RegExp:
        return (EV*) ejsCreateRegExp(ejs, ejsCreateStringFromCS(ejs, buf));
#endif

    case ES_String:
        if (strcmp(buf, "null") == 0) {
            return ejsCreateNull(ejs);

        } else if (strcmp(buf, "undefined") == 0) {
            return ejsCreateUndefined(ejs);
        }
        return ejsCreateStringFromCS(ejs, buf);
    }
    return ejsCreateUndefined(ejs);
}


/*
    Convert the variable to a number type. Only works for primitive types.
 */
static bool parseBoolean(Ejs *ejs, cchar *s)
{
    if (s == 0 || *s == '\0') {
        return 0;
    }
    if (strcmp(s, "false") == 0 || strcmp(s, "FALSE") == 0) {
        return 0;
    }
    return 1;
}


/*
    Convert the string buffer to a Number.
 */
static MprNumber parseNumber(Ejs *ejs, cchar *str)
{
    MprNumber   n;
    cchar       *cp;
    int64       num;
    int         radix, c, negative;

    mprAssert(str);

    num = 0;
    negative = 0;

    if (*str == '-') {
        str++;
        negative = 1;
    } else if (*str == '+') {
        str++;
    }

    if (*str != '.' && !isdigit((int) *str)) {
        return ejs->nanValue->value;
    }

    /*
        Floatng format: [DIGITS].[DIGITS][(e|E)[+|-]DIGITS]
     */
    if (!(*str == '0' && tolower((int) str[1]) == 'x')) {
        for (cp = str; *cp; cp++) {
            if (*cp == '.' || tolower((int) *cp) == 'e') {
                n = atof(str);
                if (negative) {
                    n = (0.0 - n);
                }
                return n;
            }
        }
    }

    /*
        Parse an integer. Observe hex and octal prefixes (0x, 0).
     */
    if (*str != '0') {
        /*
         *  Normal numbers (Radix 10)
         */
        while (isdigit((int) *str)) {
            num = (*str - '0') + (num * 10);
            str++;
        }
    } else {
        str++;
        if (tolower((int) *str) == 'x') {
            str++;
            radix = 16;
            while (*str) {
                c = tolower((int) *str);
                if (isdigit(c)) {
                    num = (c - '0') + (num * radix);
                } else if (c >= 'a' && c <= 'f') {
                    num = (c - 'a' + 10) + (num * radix);
                } else {
                    break;
                }
                str++;
            }

        } else{
            radix = 8;
            while (*str) {
                c = tolower((int) *str);
                if (isdigit(c) && c < '8') {
                    num = (c - '0') + (num * radix);
                } else {
                    break;
                }
                str++;
            }
        }
    }

    if (negative) {
        return (MprNumber) (0 - num);
    }
    return (MprNumber) num;
}


MprNumber ejsGetNumber(Ejs *ejs, EV *ev)
{
    mprAssert(ev);
    if (!ejsIsNumber(ejs, ev)) {
        if ((ev = ejsCast(ejs, ev, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, ev));
    return (ev) ? ((EjsNumber*) (ev))->value: 0;
}


bool ejsGetBoolean(Ejs *ejs, EV *ev)
{
    mprAssert(ev);
    if (!ejsIsBoolean(ejs, ev)) {
        if ((ev = ejsCast(ejs, ev, ejs->booleanType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsBoolean(ejs, ev));
    return (ev) ? ((EjsBoolean*) (ev))->value: 0;
}


int ejsGetInt(Ejs *ejs, EV *ev)
{
    mprAssert(ev);
    if (!ejsIsNumber(ejs, ev)) {
        if ((ev = ejsCast(ejs, ev, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, ev));
    return (ev) ? ((int) (((EjsNumber*) (ev))->value)): 0;
}


double ejsGetDouble(Ejs *ejs, EV *ev)
{
    mprAssert(ev);
    if (!ejsIsNumber(ejs, ev)) {
        if ((ev = ejsCast(ejs, ev, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(ejs, ev));
    return (ev) ? ((double) (((EjsNumber*) (ev))->value)): 0;
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
