/**
    ejsVar.c - Helper methods for the ejsVar interface.

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
EjsObj *ejsCast(Ejs *ejs, EjsObj *obj, EjsType *type)
{
    mprAssert(ejs);
    mprAssert(type);
    mprAssert(obj);

    if (obj == 0) {
        obj = ejs->undefinedValue;
    }
    if (obj->type == type) {
        return obj;
    }
    if (obj->type->helpers.cast) {
        return (obj->type->helpers.cast)(ejs, obj, type);
    }
    ejsThrowInternalError(ejs, "Cast helper not defined for type \"%s\"", obj->type->qname.name);
    return 0;
}


/*
    Create a new instance of a variable. Delegate to the type specific create.
 */
EjsObj *ejsCreate(Ejs *ejs, EjsType *type, int numSlots)
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
EjsObj *ejsClone(Ejs *ejs, EjsObj *obj, bool deep)
{
    EjsObj  *result;
    
    if (obj == 0) {
        return 0;
    }
    mprAssert(obj->type->helpers.clone);
    if (obj->visited == 0) {
        obj->visited = 1;
        result = (obj->type->helpers.clone)(ejs, obj, deep);
        obj->visited = 0;
    } else {
        result = obj;
    }
    return result;
}


/*
    Define a property and its traits.
    @return Return the slot number allocated for the property.
 */
int ejsDefineProperty(Ejs *ejs, EjsObj *obj, int slotNum, EjsName *name, EjsType *propType, int64 attributes, EjsObj *value)
{
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    
    mprAssert(obj->type->helpers.defineProperty);
    return (obj->type->helpers.defineProperty)(ejs, obj, slotNum, name, propType, attributes, value);
}


/**
    Delete a property in an object variable. The stack is unchanged.
    @return Returns a status code.
 */
int ejsDeleteProperty(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsType     *type;

    mprAssert(slotNum >= 0);
    
    type = obj->type;
    mprAssert(type->helpers.deleteProperty);
    return (type->helpers.deleteProperty)(ejs, obj, slotNum);
}


/**
    Delete a property in an object variable. The stack is unchanged.
    @return Returns a status code.
 */
int ejsDeletePropertyByName(Ejs *ejs, EjsObj *obj, EjsName *qname)
{
    EjsLookup   lookup;
    int         slotNum;

    mprAssert(qname);
    mprAssert(qname->name);
    mprAssert(qname->space);
    
    if (obj->type->helpers.deletePropertyByName) {
        return (obj->type->helpers.deletePropertyByName)(ejs, obj, qname);
    } else {
        slotNum = ejsLookupVar(ejs, obj, qname, &lookup);
        if (slotNum < 0) {
            ejsThrowReferenceError(ejs, "Property \"%s\" does not exist", qname->name);
            return 0;
        }
        return ejsDeleteProperty(ejs, obj, slotNum);
    }
}


void ejsDestroy(Ejs *ejs, EjsObj *obj)
{
    EjsType     *type;

    mprAssert(obj);

    type = obj->type;
    mprAssert(type->helpers.destroy);
    (type->helpers.destroy)(ejs, obj);
}


/**
    Get a property at a given slot in a variable.
    @return Returns the requested property varaible.
 */
void *ejsGetProperty(Ejs *ejs, void *vp, int slotNum)
{
    EjsType     *type;
    EjsObj      *obj;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(slotNum >= 0);

    obj = (EjsObj*) vp;
    type = obj->type;
    mprAssert(type->helpers.getProperty);
    return (type->helpers.getProperty)(ejs, obj, slotNum);
}


void *ejsGetPropertyByName(Ejs *ejs, void *vp, EjsName *name)
{
    EjsObj  *obj;
    int     slotNum;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(name);

    obj = (EjsObj*) vp;

    /*
        WARNING: this is not implemented by most types
     */
    if (obj->type->helpers.getPropertyByName) {
        return (obj->type->helpers.getPropertyByName)(ejs, obj, name);
    }

    /*
     *  Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, obj, name);
    if (slotNum < 0) {
        return 0;
    }
    return ejsGetProperty(ejs, obj, slotNum);
}


/**
    Return the number of properties in the variable.
    @return Returns the number of properties.
 */
int ejsGetPropertyCount(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj->type->helpers.getPropertyCount);
    return (obj->type->helpers.getPropertyCount)(ejs, obj);
}


/**
    Return the name of a property indexed by slotNum.
    @return Returns the property name.
 */
EjsName ejsGetPropertyName(Ejs *ejs, void *vp, int slotNum)
{
    EjsType     *type;
    EjsObj      *obj;

    obj = (EjsObj*) vp;

    type = obj->type;
    mprAssert(type->helpers.getPropertyName);
    return (type->helpers.getPropertyName)(ejs, obj, slotNum);
}


#if UNUSED
/**
    Return the trait for the indexed by slotNum.
    @return Returns the property name.
 */
EjsTrait *ejsGetPropertyTrait(Ejs *ejs, EjsObj *obj, int slotNum)
{
    EjsType     *type;

    type = obj->type;
    mprAssert(type->helpers.getPropertyTrait);
    return (type->helpers.getPropertyTrait)(ejs, obj, slotNum);
}
#endif


/**
    Get a property slot. Lookup a property name and return the slot reference. If a namespace is supplied, the property
    must be defined with the same namespace.
    @return Returns the slot number or -1 if it does not exist.
 */
int ejsLookupProperty(Ejs *ejs, EjsObj *obj, EjsName *name)
{
    mprAssert(ejs);
    mprAssert(obj);
    mprAssert(name);
    mprAssert(name->name);

    mprAssert(obj->type->helpers.lookupProperty);
    return (obj->type->helpers.lookupProperty)(ejs, obj, name);
}


/*
    Invoke an operator.
    obj is left-hand-side
    @return Return a variable with the result or null if an exception is thrown.
 */
EjsObj *ejsInvokeOperator(Ejs *ejs, EjsObj *obj, int opCode, EjsObj *rhs)
{
    mprAssert(obj);

    mprAssert(obj->type->helpers.invokeOperator);
    return (obj->type->helpers.invokeOperator)(ejs, obj, opCode, rhs);
}


/*
    ejsMark is in ejsGarbage.c
 */


/*
    Set a property and return the slot number. Incoming slot may be -1 to allocate a new slot.
 */
int ejsSetProperty(Ejs *ejs, void *vp, int slotNum, void *value)
{
    EjsObj      *obj;

    mprAssert(vp);
    obj = (EjsObj*) vp;

    if (obj == 0) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    mprAssert(obj->type->helpers.setProperty);
    return (obj->type->helpers.setProperty)(ejs, obj, slotNum, value);
}


/*
    Set a property given a name.
 */
int ejsSetPropertyByName(Ejs *ejs, void *vp, EjsName *qname, void *value)
{
    EjsObj  *obj;
    int     slotNum;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(qname);

    obj = (EjsObj*) vp;

    /*
     *  WARNING: Not all types implement this
     */
    if (obj->type->helpers.setPropertyByName) {
        return (obj->type->helpers.setPropertyByName)(ejs, obj, qname, value);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, obj, qname);
    if (slotNum < 0) {
        slotNum = ejsSetProperty(ejs, obj, -1, value);
        if (slotNum < 0) {
            return EJS_ERR;
        }
        if (ejsSetPropertyName(ejs, obj, slotNum, qname) < 0) {
            return EJS_ERR;
        }
        return slotNum;
    }
    return ejsSetProperty(ejs, obj, slotNum, value);
}


/*
    Set the property name and return the slot number. Slot may be -1 to allocate a new slot.
 */
int ejsSetPropertyName(Ejs *ejs, EjsObj *obj, int slot, EjsName *qname)
{
    mprAssert(obj->type->helpers.setPropertyName);
    return (obj->type->helpers.setPropertyName)(ejs, obj, slot, qname);
}


/**
    Get a string representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToString(Ejs *ejs, EjsObj *obj)
{
    return (EjsString*) ejsCast(ejs, obj, ejs->stringType);
}


/**
    Get a numeric representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsNumber *ejsToNumber(Ejs *ejs, EjsObj *obj)
{
    if (obj == 0 || ejsIsNumber(obj)) {
        return (EjsNumber*) obj;
    }
    if (obj->type->helpers.cast) {
        return (EjsNumber*) (obj->type->helpers.cast)(ejs, obj, ejs->numberType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", obj->type->qname.name);
    return 0;
}


/**
    Get a boolean representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsBoolean *ejsToBoolean(Ejs *ejs, EjsObj *obj)
{
    if (obj == 0 || ejsIsBoolean(obj)) {
        return (EjsBoolean*) obj;
    }
    if (obj->type->helpers.cast) {
        return (EjsBoolean*) (obj->type->helpers.cast)(ejs, obj, ejs->booleanType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", obj->type->qname.name);
    return 0;
}


/**
    Get a serialized string representation of a variable using JSON encoding.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToJSON(Ejs *ejs, EjsObj *obj, EjsObj *options)
{
    EjsFunction     *fn;
    EjsString       *result;
    EjsObj          *argv[1];
    EjsName         qname;
    int             argc;

    if (obj == 0) {
        return ejsCreateString(ejs, "undefined");
    }
    fn = (EjsFunction*) ejsGetPropertyByName(ejs, (EjsObj*) obj->type->prototype, ejsName(&qname, NULL, "toJSON"));
    if (fn == 0) {
        fn = (EjsFunction*) ejsGetPropertyByName(ejs, (EjsObj*) ejs->objectType->prototype, &qname);        
    }
    if (ejsIsFunction(fn)) {
        if (options) {
            argc = 1;
            argv[0] = options;
        } else {
            argc = 0;
            argv[0] = NULL;
        }
        result = (EjsString*) ejsRunFunction(ejs, fn, obj, argc, argv);
    } else {
        result = ejsToString(ejs, obj);
    }
    return result;
}


/*
    Fully construct a new object. We create a new instance and call all required constructors.
 */
EjsObj *ejsCreateInstance(Ejs *ejs, EjsType *type, int argc, EjsObj **argv)
{
    EjsObj      *obj;

    mprAssert(type);

    obj = ejsCreate(ejs, type, 0);
    if (obj == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (type->constructor.block.obj.isFunction) {
        obj->permanent = 1;
        ejsRunFunction(ejs, (EjsFunction*) type, obj, argc, argv);
        obj->permanent = 0;
    }
    return obj;
}



int _ejsIs(EjsObj *obj, int slot)
{
    EjsType     *tp;

    if (obj == 0) {
        return 0;
    }
    if (obj->type->id == slot) {
        return 1;
    }
    for (tp = ((EjsObj*) obj)->type->baseType; tp; tp = tp->baseType) {
        if (tp->id == slot) {
            return 1;
        }
    }
    return 0;
}

/************************************* Misc ***********************************/

EjsName *ejsAllocName(MprCtx ctx, cchar *name, cchar *space)
{
    EjsName     *np;

    np = mprAllocObj(ctx, EjsName);
    if (np) {
        np->name = mprStrdup(np, name);
        np->space = mprStrdup(np, space);
    }
    return np;
}


EjsName ejsCopyName(MprCtx ctx, EjsName *qname)
{
    EjsName     name;

    name.name = mprStrdup(ctx, qname->name);
    name.space = mprStrdup(ctx, qname->space);
    return name;
}


EjsName *ejsDupName(MprCtx ctx, EjsName *qname)
{
    return ejsAllocName(ctx, qname->name, qname->space);
}


EjsName *ejsName(EjsName *np, cchar *space, cchar *name)
{
    np->name = name;
    np->space = space;
    return np;
}


/*
    Parse a string based on formatting instructions and intelligently create a variable.
    Number formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]
        [+|-][DIGITS][.][DIGITS][(e|E)[+|-]DIGITS]
 */
EjsObj *ejsParse(Ejs *ejs, cchar *buf, int preferredType)
{
    int         type;

    mprAssert(buf);

    type = preferredType;

    if (preferredType == ES_Void || preferredType < 0) {
        if (*buf == '-' || *buf == '+') {
            type = ejs->numberType->id;

        } else if (*buf == '/') {
            type = ES_RegExp;

        } else if (!isdigit((int) *buf) && *buf != '.') {
            if (strcmp(buf, "true") == 0) {
                return (EjsVar*) ejs->trueValue;

            } else if (strcmp(buf, "false") == 0) {
                return (EjsVar*) ejs->falseValue;
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
        return (EjsObj*) ejsCreateNumber(ejs, parseNumber(ejs, buf));

    case ES_Boolean:
        return (EjsObj*) ejsCreateBoolean(ejs, parseBoolean(ejs, buf));

    case ES_RegExp:
        return (EjsObj*) ejsCreateRegExp(ejs, buf);

    case ES_String:
        if (strcmp(buf, "null") == 0) {
            return (EjsObj*) ejsCreateNull(ejs);

        } else if (strcmp(buf, "undefined") == 0) {
            return (EjsObj*) ejsCreateUndefined(ejs);
        }
        return (EjsObj*) ejsCreateString(ejs, buf);
    }
    return (EjsObj*) ejsCreateUndefined(ejs);
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


MprNumber _ejsGetNumber(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    if (!ejsIsNumber(obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(obj));
    return (obj) ? ((EjsNumber*) (obj))->value: 0;
}


int _ejsGetBoolean(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    if (!ejsIsBoolean(obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->booleanType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsBoolean(obj));
    return (obj) ? ((EjsBoolean*) (obj))->value: 0;
}


int _ejsGetInt(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    if (!ejsIsNumber(obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(obj));
    return (obj) ? ((int) (((EjsNumber*) (obj))->value)): 0;
}


double _ejsGetDouble(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    if (!ejsIsNumber(obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(obj));
    return (obj) ? ((double) (((EjsNumber*) (obj))->value)): 0;
}


cchar *_ejsGetString(Ejs *ejs, EjsObj *obj)
{
    mprAssert(obj);
    if (!ejsIsString(obj)) {
        if ((obj = ejsCast(ejs, obj, ejs->stringType)) == 0) {
            return "";
        }
    }
    mprAssert(ejsIs(obj, ES_String));
    return (obj) ? (((EjsString*) obj)->value): "";
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
