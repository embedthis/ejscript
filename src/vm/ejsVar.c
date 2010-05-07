/**
    ejsVar.c - Helper methods for the ejsVar interface.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/****************************** Forward Declarations **************************/

static MprNumber parseNumber(Ejs *ejs, cchar *str);
static bool      parseBoolean(Ejs *ejs, cchar *s);

/*
    Get the type owning a property
    MOB -- remove this function when have tested script over native
 */
static inline EjsType *getOwningType(EjsObj *vp, int slotNum)
{
    EjsType     *type;

    type = vp->type;

#if UNUSED && MOB
    if (type->instanceSize != sizeof(EjsObject)) {
        if (vp->isType) {
            if (slotNum < type->numInherited) {
                do {
                    type = type->baseType;
                } while (slotNum < type->numInherited);
            }
        } else if (type->prototype) {
            if (slotNum < type->numPrototypeInherited) {
                do {
                    type = type->baseType;
                } while (slotNum < type->numPrototypeInherited);
            }
        }
    }
#endif
    return type;
}

/************************************* Code ***********************************/
/**
    Cast the variable to a given target type.
    @return Returns a variable with the result of the cast or null if an exception is thrown.
 */
EjsObj *ejsCast(Ejs *ejs, EjsObj *vp, EjsType *type)
{
    EjsFunction     *fun;
    EjsObj          *prototype;

    mprAssert(ejs);
    mprAssert(type);

    if (vp == 0) {
        vp = ejs->undefinedValue;
    }
    if (vp->type == type) {
        return vp;
    }
#if FUTURE
    EjsName         qname;
    if (type->hasMeta) {
        return ejsRunFunctionByName(ejs, (EjsObj*) type, ejsName(&qname, EJS_META_NAMESPACE, "cast"), 
            (EjsObj*) type, 1, &vp);
        if ((slotNum = ejsLookupProperty(ejs, (EjsObj*) type, ejsName(&qname, EJS_META_NAMESPACE, "cast"))) >= 0) {
            type->hasMeta
        }
    }
#endif
    if (type == ejs->stringType) {
        if (vp == 0) {
            return (EjsObj*) ejsCreateString(ejs, "undefined");
        } else if (ejsIsString(vp)) {
            return (EjsObj*) vp;
        }
        prototype = vp->type->prototype;
        //  MOB BUG This should use ejsLookupVar
        if (prototype->numSlots >= ES_Object_toString) {
            if (ejsGetTraitAttributes((EjsObj*) prototype, ES_Object_toString) & EJS_FUN_OVERRIDE) {
                fun = (EjsFunction*) ejsGetProperty(ejs, prototype, ES_Object_toString);
                return (EjsObj*) ejsRunFunction(ejs, fun, vp, 0, NULL);
            }
        }
    }
    if (vp->type->helpers.cast) {
        return (vp->type->helpers.cast)(ejs, vp, type);
    }
    ejsThrowInternalError(ejs, "Cast helper not defined for type \"%s\"", vp->type->qname.name);
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
EjsObj *ejsClone(Ejs *ejs, EjsObj *vp, bool deep)
{
    EjsObj  *result;
    
    if (vp == 0) {
        return 0;
    }
    mprAssert(vp->type->helpers.clone);
    if (vp->visited == 0) {
        vp->visited = 1;
        result = (vp->type->helpers.clone)(ejs, vp, deep);
        vp->visited = 0;
    } else {
        result = vp;
    }
    return result;
}


/*
    Define a property and its traits.
    @return Return the slot number allocated for the property.
 */
int ejsDefineProperty(Ejs *ejs, EjsObj *vp, int slotNum, EjsName *name, EjsType *propType, int attributes, EjsObj *value)
{
    mprAssert(name);
    mprAssert(name->name);
    mprAssert(name->space);
    
    mprAssert(vp->type->helpers.defineProperty);
    return (vp->type->helpers.defineProperty)(ejs, vp, slotNum, name, propType, attributes, value);
}


/**
    Delete a property in an object variable. The stack is unchanged.
    @return Returns a status code.
 */
int ejsDeleteProperty(Ejs *ejs, EjsObj *vp, int slotNum)
{
    EjsType     *type;

    mprAssert(slotNum >= 0);
    
    type = getOwningType(vp, slotNum);
    mprAssert(type->helpers.deleteProperty);
    return (type->helpers.deleteProperty)(ejs, vp, slotNum);
}


/**
    Delete a property in an object variable. The stack is unchanged.
    @return Returns a status code.
 */
int ejsDeletePropertyByName(Ejs *ejs, EjsObj *vp, EjsName *qname)
{
    EjsLookup   lookup;
    int         slotNum;

    mprAssert(qname);
    mprAssert(qname->name);
    mprAssert(qname->space);
    
    if (vp->type->helpers.deletePropertyByName) {
        return (vp->type->helpers.deletePropertyByName)(ejs, vp, qname);
    } else {
        slotNum = ejsLookupVar(ejs, vp, qname, &lookup);
        if (slotNum < 0) {
            ejsThrowReferenceError(ejs, "Property \"%s\" does not exist", qname->name);
            return 0;
        }
        return ejsDeleteProperty(ejs, vp, slotNum);
    }
}


void ejsDestroy(Ejs *ejs, EjsObj *vp)
{
    EjsType     *type;

    mprAssert(vp);

    type = vp->type;
    mprAssert(type->helpers.destroy);
    (type->helpers.destroy)(ejs, vp);
}


/**
    Get a property at a given slot in a variable.
    @return Returns the requested property varaible.
 */
EjsObj *ejsGetProperty(Ejs *ejs, EjsObj *vp, int slotNum)
{
    EjsType     *type;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(slotNum >= 0);

    type = getOwningType(vp, slotNum);
    mprAssert(type->helpers.getProperty);
    return (type->helpers.getProperty)(ejs, vp, slotNum);
}


/*
    Get a property given a name.
 */
EjsObj *ejsGetPropertyByName(Ejs *ejs, EjsObj *vp, EjsName *name)
{
    int     slotNum;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(name);

    /*
     *  WARNING: this is not implemented by most types
     */
    if (vp->type->helpers.getPropertyByName) {
        return (vp->type->helpers.getPropertyByName)(ejs, vp, name);
    }

    /*
     *  Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, vp, name);
    if (slotNum < 0) {
        return 0;
    }
    return ejsGetProperty(ejs, vp, slotNum);
}


/**
    Return the number of properties in the variable.
    @return Returns the number of properties.
 */
int ejsGetPropertyCount(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp->type->helpers.getPropertyCount);
    return (vp->type->helpers.getPropertyCount)(ejs, vp);
}


/**
    Return the name of a property indexed by slotNum.
    @return Returns the property name.
 */
EjsName ejsGetPropertyName(Ejs *ejs, EjsObj *vp, int slotNum)
{
    EjsType     *type;

    type = getOwningType(vp, slotNum);
    mprAssert(type->helpers.getPropertyName);
    return (type->helpers.getPropertyName)(ejs, vp, slotNum);
}


/**
    Return the trait for the indexed by slotNum.
    @return Returns the property name.
 */
EjsTrait *ejsGetPropertyTrait(Ejs *ejs, EjsObj *vp, int slotNum)
{
    EjsType     *type;

    type = getOwningType(vp, slotNum);
    mprAssert(type->helpers.getPropertyTrait);
    return (type->helpers.getPropertyTrait)(ejs, vp, slotNum);
}


/**
    Get a property slot. Lookup a property name and return the slot reference. If a namespace is supplied, the property
    must be defined with the same namespace.
    @return Returns the slot number or -1 if it does not exist.
 */
int ejsLookupProperty(Ejs *ejs, EjsObj *vp, EjsName *name)
{
    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(name);
    mprAssert(name->name);

    mprAssert(vp->type->helpers.lookupProperty);
    return (vp->type->helpers.lookupProperty)(ejs, vp, name);
}


/*
    Invoke an operator.
    vp is left-hand-side
    @return Return a variable with the result or null if an exception is thrown.
 */
EjsObj *ejsInvokeOperator(Ejs *ejs, EjsObj *vp, int opCode, EjsObj *rhs)
{
    mprAssert(vp);

    mprAssert(vp->type->helpers.invokeOperator);
    return (vp->type->helpers.invokeOperator)(ejs, vp, opCode, rhs);
}


/*
    ejsMark is in ejsGarbage.c
 */


/*
    Set a property and return the slot number. Incoming slot may be -1 to allocate a new slot.
 */
int ejsSetProperty(Ejs *ejs, EjsObj *vp, int slotNum, EjsObj *value)
{
    mprAssert(vp);
    if (vp == 0) {
        ejsThrowReferenceError(ejs, "Object is null");
        return EJS_ERR;
    }
    mprAssert(vp->type->helpers.setProperty);
    return (vp->type->helpers.setProperty)(ejs, vp, slotNum, value);
}


/*
    Set a property given a name.
 */
int ejsSetPropertyByName(Ejs *ejs, EjsObj *vp, EjsName *qname, EjsObj *value)
{
    int     slotNum;

    mprAssert(ejs);
    mprAssert(vp);
    mprAssert(qname);

    /*
     *  WARNING: Not all types implement this
     */
    if (vp->type->helpers.setPropertyByName) {
        return (vp->type->helpers.setPropertyByName)(ejs, vp, qname, value);
    }

    /*
        Fall back and use a two-step lookup and get
     */
    slotNum = ejsLookupProperty(ejs, vp, qname);
    if (slotNum < 0) {
        slotNum = ejsSetProperty(ejs, vp, -1, value);
        if (slotNum < 0) {
            return EJS_ERR;
        }
        if (ejsSetPropertyName(ejs, vp, slotNum, qname) < 0) {
            return EJS_ERR;
        }
        return slotNum;
    }
    return ejsSetProperty(ejs, vp, slotNum, value);
}


/*
    Set the property name and return the slot number. Slot may be -1 to allocate a new slot.
 */
int ejsSetPropertyName(Ejs *ejs, EjsObj *vp, int slot, EjsName *qname)
{
    mprAssert(vp->type->helpers.setPropertyName);
    return (vp->type->helpers.setPropertyName)(ejs, vp, slot, qname);
}


int ejsSetPropertyTrait(Ejs *ejs, EjsObj *vp, int slot, EjsType *propType, int attributes)
{
    mprAssert(vp->type->helpers.setPropertyTrait);
    return (vp->type->helpers.setPropertyTrait)(ejs, vp, slot, propType, attributes);
}


/**
    Get a string representation of a variable.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToString(Ejs *ejs, EjsObj *vp)
{
    return (EjsString*) ejsCast(ejs, vp, ejs->stringType);
#if OLD
    EjsFunction     *fn;

    if (vp == 0) {
        return ejsCreateString(ejs, "undefined");
    } else if (ejsIsString(vp)) {
        return (EjsString*) vp;
    }
    if (vp->type->block.obj.numSlots >= ES_Object_toString) {
        fn = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) vp->type, ES_Object_toString);
        if (ejsIsFunction(fn)) {
            return (EjsString*) ejsRunFunction(ejs, fn, vp, 0, NULL);
        }
    }
    if (vp->type->helpers.cast) {
        return (EjsString*) (vp->type->helpers.cast)(ejs, vp, ejs->stringType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", vp->type->qname.name);
    return 0;
#endif
}


/**
    Get a numeric representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsNumber *ejsToNumber(Ejs *ejs, EjsObj *vp)
{
    if (vp == 0 || ejsIsNumber(vp)) {
        return (EjsNumber*) vp;
    }
    if (vp->type->helpers.cast) {
        return (EjsNumber*) (vp->type->helpers.cast)(ejs, vp, ejs->numberType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", vp->type->qname.name);
    return 0;
}


/**
    Get a boolean representation of a variable.
    @return Returns a number variable or null if an exception is thrown.
 */
EjsBoolean *ejsToBoolean(Ejs *ejs, EjsObj *vp)
{
    if (vp == 0 || ejsIsBoolean(vp)) {
        return (EjsBoolean*) vp;
    }
    if (vp->type->helpers.cast) {
        return (EjsBoolean*) (vp->type->helpers.cast)(ejs, vp, ejs->booleanType);
    }
    ejsThrowInternalError(ejs, "CastVar helper not defined for type \"%s\"", vp->type->qname.name);
    return 0;
}


/**
    Get a serialized string representation of a variable using JSON encoding.
    @return Returns a string variable or null if an exception is thrown.
 */
EjsString *ejsToJSON(Ejs *ejs, EjsObj *vp, EjsObj *options)
{
    EjsFunction     *fn;
    EjsString       *result;
    EjsObj          *argv[1];
    int             argc;

    if (vp == 0) {
        return ejsCreateString(ejs, "undefined");
    }
    if (vp->jsonVisited) {
        return ejsCreateString(ejs, "this");
    }    
    vp->jsonVisited = 1;
    
    fn = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) vp->type->prototype, ES_Object_toJSON);
    if (ejsIsFunction(fn)) {
        if (options) {
            argc = 1;
            argv[0] = options;
        } else {
            argc = 0;
            argv[0] = NULL;
        }
        result = (EjsString*) ejsRunFunction(ejs, fn, vp, argc, argv);
    } else {
        result = ejsToString(ejs, vp);
    }
    vp->jsonVisited = 0;
    return result;
}


/*
    Fully construct a new object. We create a new instance and call all required constructors.
 */
EjsObj *ejsCreateInstance(Ejs *ejs, EjsType *type, int argc, EjsObj **argv)
{
    EjsFunction     *fun;
    EjsObj          *vp;

    mprAssert(type);

    vp = ejsCreate(ejs, type, 0);
    if (vp == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    if (type->hasConstructor) {
        mprAssert(type->hasInitializer);
        fun = (EjsFunction*) ejsGetProperty(ejs, (EjsObj*) type->prototype, type->numPrototypeInherited);
        if (fun == 0 || !ejsIsFunction(fun)) {
            return 0;
        }
        vp->permanent = 1;
        ejsRunFunction(ejs, fun, vp, argc, argv);
        vp->permanent = 0;
    } else {
        mprAssert(!type->hasInitializer);
    }
    return vp;
}



int _ejsIs(EjsObj *vp, int slot)
{
    EjsType     *tp;

    if (vp == 0) {
        return 0;
    }
    if (vp->type->id == slot) {
        return 1;
    }
    for (tp = ((EjsObj*) vp)->type->baseType; tp; tp = tp->baseType) {
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


MprNumber _ejsGetNumber(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(vp));
    return (vp) ? ((EjsNumber*) (vp))->value: 0;
}


int _ejsGetBoolean(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp);
    if (!ejsIsBoolean(vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->booleanType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsBoolean(vp));
    return (vp) ? ((EjsBoolean*) (vp))->value: 0;
}


int _ejsGetInt(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(vp));
    return (vp) ? ((int) (((EjsNumber*) (vp))->value)): 0;
}


double _ejsGetDouble(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp);
    if (!ejsIsNumber(vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->numberType)) == 0) {
            return 0;
        }
    }
    mprAssert(ejsIsNumber(vp));
    return (vp) ? ((double) (((EjsNumber*) (vp))->value)): 0;
}


cchar *_ejsGetString(Ejs *ejs, EjsObj *vp)
{
    mprAssert(vp);
    if (!ejsIsString(vp)) {
        if ((vp = ejsCast(ejs, vp, ejs->stringType)) == 0) {
            return "";
        }
    }
    mprAssert(ejsIs(vp, ES_String));
    return (vp) ? (((EjsString*) vp)->value): "";
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
