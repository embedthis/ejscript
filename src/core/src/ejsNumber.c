/**
    ejsNumber.c - Number type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/**************************** Forward Declarations ****************************/

#define fixed(n) ((int64) (floor(n)))

/******************************************************************************/
/*
    Cast the operand to the specified type
 */
static EjsAny *castNumber(Ejs *ejs, EjsNumber *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ((vp->value) ? S(true) : S(false));

    case S_String:
        //  OPT. mprDtoa does a sclone.
        return ejsCreateStringFromAsc(ejs, mprDtoa(vp->value, 0, 0, 0));

    case S_Number:
        return vp;
            
    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsNumber *cloneNumber(Ejs *ejs, EjsNumber *np, int deep)
{
    return ejsCreateNumber(ejs, np->value);
}


static EjsAny *coerceNumberOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs)
{
    switch (opcode) {
    /*
        Binary operators
     */
    case EJS_OP_ADD:
        if (ejsIs(ejs, rhs, Void)) {
            return S(nan);
        } else if (ejsIs(ejs, rhs, Null)) {
            return lhs;
        } else if (ejsIs(ejs, rhs, Boolean) || ejsIs(ejs, rhs, Date)) {
            return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));
        } else {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opcode == EJS_OP_COMPARE_EQ) ? S(false): S(true));
        } else if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        } else if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_STRICTLY_NE:
        return S(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return S(false);

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (((EjsNumber*) lhs)->value ? S(true) : S(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsNumber*) lhs)->value ? S(false): S(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return S(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return S(undefined);
    }
    return 0;
}


static EjsAny *invokeNumberOperator(Ejs *ejs, EjsNumber *lhs, int opcode, EjsNumber *rhs)
{
    EjsObj      *result;

    mprAssert(lhs);
    
    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIs(ejs, lhs, Number) || !ejsIs(ejs, rhs, Number)) {
            if ((result = coerceNumberOperands(ejs, lhs, opcode, rhs)) != 0) {
                return result;
            }
        }
    }

    /*
        Types now match, both numbers
     */
    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
        return ((lhs->value == rhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
        return ((lhs->value != rhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_LT:
        return ((lhs->value < rhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_LE:
        return ((lhs->value <= rhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_GT:
        return ((lhs->value > rhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_GE:
        return ((lhs->value >= rhs->value) ? S(true): S(false));

    /*
     *  Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->value) ? S(true): S(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->value == 0) ? S(true): S(false));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return S(false);

    case EJS_OP_NEG:
        return ejsCreateNumber(ejs, -lhs->value);

    case EJS_OP_LOGICAL_NOT:
        return ejsCreateBoolean(ejs, !fixed(lhs->value));

    case EJS_OP_NOT:
        return ejsCreateNumber(ejs, (MprNumber) (~fixed(lhs->value)));


    /*
        Binary operations
     */
    case EJS_OP_ADD:
        return ejsCreateNumber(ejs, lhs->value + rhs->value);

    case EJS_OP_AND:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) & fixed(rhs->value)));

    case EJS_OP_DIV:
        return ejsCreateNumber(ejs, lhs->value / rhs->value);

    case EJS_OP_MUL:
        return ejsCreateNumber(ejs, lhs->value * rhs->value);

    case EJS_OP_OR:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) | fixed(rhs->value)));

    case EJS_OP_REM:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) % fixed(rhs->value)));

    case EJS_OP_SHL:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) << fixed(rhs->value)));

    case EJS_OP_SHR:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_SUB:
        return ejsCreateNumber(ejs, lhs->value - rhs->value);

    case EJS_OP_USHR:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_XOR:
        return ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) ^ fixed(rhs->value)));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
}


/*********************************** Methods **********************************/
/*
    Number constructor.
    function Number(value: Object = null)
 */
static EjsNumber *numberConstructor(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    EjsNumber   *num;

    mprAssert(argc == 0 || argc == 1);

    if (argc == 1) {
        num = ejsToNumber(ejs, argv[0]);
        if (num) {
            np->value = num->value;
        }
    }
    return np;
}


/*
    Function to iterate and return each number in sequence.
    NOTE: this is not a method of Number. Rather, it is a callback function for Iterator.
 */
static EjsNumber *nextNumber(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsNumber   *np;

    np = (EjsNumber*) ip->target;
    if (!ejsIs(ejs, np, Number)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < np->value) {
        return ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    function integral(size: Number: 32): Number
 */
static EjsNumber *integral(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    int64   mask, result;
    int     size;

    size = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 32;

    result = ((int64) np->value);
    if (size < 64) {
        mask = 1;
        mask = (mask << size) - 1;
        result &= mask;
    }
    return ejsCreateNumber(ejs, (MprNumber) result);
}


/*
    function get isFinite(): Boolean
 */
static EjsBoolean *isFinite(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    if (np->value == ((EjsNumber*) S(nan))->value || np->value == ((EjsNumber*) S(infinity))->value || 
            np->value == ((EjsNumber*) S(negativeInfinity))->value) {
        return S(false);
    }
    return S(true);
}


/*
    function get isNaN(): Boolean
 */
static EjsBoolean *isNaN(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    return (mprIsNan(np->value) ? S(true) : S(false));
}


/*
    function toExponential(fractionDigits: Number = 0): String

    Display with only one digit before the decimal point.
 */
static EjsString *toExponential(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]): 0;
    result = mprDtoa(np->value, ndigits, MPR_DTOA_N_DIGITS, MPR_DTOA_EXPONENT_FORM);
    return ejsCreateStringFromAsc(ejs, result);
}


/*
    function toFixed(fractionDigits: Number = 0): String

    Display the specified number of fractional digits
 */
static EjsString *toFixed(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 0;
    result = mprDtoa(np->value, ndigits, MPR_DTOA_N_FRACTION_DIGITS, MPR_DTOA_FIXED_FORM);
    return ejsCreateStringFromAsc(ejs, result);
}


/*
    function toPrecision(numDigits: Number = MAX_VALUE): String

    Display the specified number of total digits
 */
static EjsString *toPrecision(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 0;
    result = mprDtoa(np->value, ndigits, MPR_DTOA_N_DIGITS, 0);
    return ejsCreateStringFromAsc(ejs, result);
}


/*
    Return the default iterator. This returns the index names.

    iterator native function get(): Iterator
 */
static EjsIterator *getNumberIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, np, nextNumber, 0, NULL);
}


/*
    Convert the number to a string.
    function toString(): String
 */
static EjsObj *numberToString(Ejs *ejs, EjsNumber *vp, int argc, EjsObj **argv)
{
    return castNumber(ejs, vp, S(String));
}


/*********************************** Support **********************************/

#ifndef ejsIsNan
int ejsIsNan(double f)
{
#if BLD_WIN_LIKE
    return _isnan(f);
#elif VXWORKS
    /* TODO */
    return 0;
#else
    return (f == FP_NAN);
#endif
}
#endif


bool ejsIsInfinite(MprNumber f)
{
#if BLD_WIN_LIKE
    return !_finite(f);
#elif VXWORKS
    /* TODO */
    return 0;
#else
    return (f == FP_INFINITE);
#endif
}

/*********************************** Factory **********************************/
/*
    Create an initialized number
 */

EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value)
{
    EjsNumber   *vp;

    if (value == 0) {
        return S(zero);
    } else if (value == 1) {
        return S(one);
    } else if (value == -1) {
        return S(minusOne);
    }
    if ((vp = ejsCreateObj(ejs, S(Number), 0)) != 0) {
        vp->value = value;
    }
    return vp;
}


void ejsCreateNumberType(Ejs *ejs)
{
    EjsNumber   *np;
    EjsType     *type;
    static int  zero = 0;

    type = ejsCreateNativeType(ejs, N("ejs", "Number"), sizeof(EjsNumber), S_Number, ES_Number_NUM_CLASS_PROP, 
        NULL, EJS_OBJ_HELPERS);

    type->helpers.cast = (EjsCastHelper) castNumber;
    type->helpers.clone = (EjsCloneHelper) cloneNumber;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeNumberOperator;

    np = ejsCreateObj(ejs, type, 0);
    np->value = 0;
    ejsSetSpecial(ejs, S_zero, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1;
    ejsSetSpecial(ejs, S_one, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = -1;
    ejsSetSpecial(ejs, S_minusOne, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1.0 / zero;
    ejsSetSpecial(ejs, S_infinity, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = -1.0 / zero;
    ejsSetSpecial(ejs, S_negativeInfinity, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 0.0 / zero;
    ejsSetSpecial(ejs, S_nan, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1.7976931348623157e+308;
    ejsSetSpecial(ejs, S_max, np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 5e-324;
    ejsSetSpecial(ejs, S_min, np);
}


void ejsConfigureNumberType(Ejs *ejs)
{
    EjsType    *type;
    EjsPot     *prototype;

    type = S(Number);
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, numberConstructor);
    ejsBindMethod(ejs, prototype, ES_Number_iterator_get, getNumberIterator);
    ejsBindMethod(ejs, prototype, ES_Number_iterator_getValues, getNumberIterator);
    ejsBindMethod(ejs, prototype, ES_Number_integral, integral);
    ejsBindMethod(ejs, prototype, ES_Number_isFinite, isFinite);
    ejsBindMethod(ejs, prototype, ES_Number_isNaN, isNaN);
    ejsBindMethod(ejs, prototype, ES_Number_toExponential, toExponential);
    ejsBindMethod(ejs, prototype, ES_Number_toFixed, toFixed);
    ejsBindMethod(ejs, prototype, ES_Number_toPrecision, toPrecision);
    ejsBindMethod(ejs, prototype, ES_Number_toString, numberToString);

    ejsSetProperty(ejs, type, ES_Number_MaxValue, S(max));
    ejsSetProperty(ejs, type, ES_Number_MinValue, S(min));
    ejsSetProperty(ejs, type, ES_Number_NEGATIVE_INFINITY, S(negativeInfinity));
    ejsSetProperty(ejs, type, ES_Number_POSITIVE_INFINITY, S(infinity));
    ejsSetProperty(ejs, type, ES_Number_NaN, S(nan));

    ejsSetProperty(ejs, ejs->global, ES_NegativeInfinity, S(negativeInfinity));
    ejsSetProperty(ejs, ejs->global, ES_Infinity, S(infinity));
    ejsSetProperty(ejs, ejs->global, ES_NaN, S(nan));
    ejsSetProperty(ejs, ejs->global, ES_double, type);
    ejsSetProperty(ejs, ejs->global, ES_num, type);
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
