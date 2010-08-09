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
static EjsObj *castNumber(Ejs *ejs, EjsNumber *vp, EjsType *type)
{
    char    *result;
    
    switch (type->id) {
    case ES_Boolean:
        return (EjsObj*) ((vp->value) ? ejs->trueValue : ejs->falseValue);

    case ES_String:
        result = mprDtoa(vp, vp->value, 0, 0, 0);
        return (EjsObj*) ejsCreateStringAndFree(ejs, result);

    case ES_Number:
        return (EjsObj*) vp;
            
    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsObj *cloneNumber(Ejs *ejs, EjsNumber *np, int deep)
{
    if (deep) {
        return (EjsObj*) ejsCreateNumber(ejs, np->value);
    }
    return (EjsObj*) np;
}


static EjsObj *coerceNumberOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {
    /*
     *  Binary operators
     */
    case EJS_OP_ADD:
        if (ejsIsUndefined(rhs)) {
            return (EjsObj*) ejs->nanValue;
        } else if (ejsIsNull(rhs)) {
            return (EjsObj*) lhs;
        } else if (ejsIsBoolean(rhs) || ejsIsDate(rhs)) {
            return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToNumber(ejs, rhs));
        } else {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIsNull(rhs) || ejsIsUndefined(rhs)) {
            return (EjsObj*) ((opcode == EJS_OP_COMPARE_EQ) ? ejs->falseValue: ejs->trueValue);
        } else if (ejsIsNumber(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToNumber(ejs, lhs), opcode, rhs);
        } else if (ejsIsString(rhs)) {
            return ejsInvokeOperator(ejs, (EjsObj*) ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, lhs, opcode, (EjsObj*) ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ejs->trueValue;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ejs->falseValue;

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) (((EjsNumber*) lhs)->value ? ejs->trueValue : ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (EjsObj*) (((EjsNumber*) lhs)->value ? ejs->falseValue: ejs->trueValue);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return (EjsObj*) ejs->falseValue;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %s", opcode, lhs->type->qname.name);
        return ejs->undefinedValue;
    }
    return 0;
}


static EjsObj *invokeNumberOperator(Ejs *ejs, EjsNumber *lhs, int opcode, EjsNumber *rhs)
{
    EjsObj      *result;

    mprAssert(lhs);
    
    //  TODO - why test rhs == 0
    if (rhs == 0 || lhs->obj.type != rhs->obj.type) {
        if (!ejsIsA(ejs, (EjsObj*) lhs, ejs->numberType) || !ejsIsA(ejs, (EjsObj*) rhs, ejs->numberType)) {
            if ((result = coerceNumberOperands(ejs, (EjsObj*) lhs, opcode, (EjsObj*) rhs)) != 0) {
                return result;
            }
        }
    }

    /*
        Types now match, both numbers
     */
    switch (opcode) {

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
        return (EjsObj*) ((lhs->value == rhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
        return (EjsObj*) ((lhs->value != rhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_LT:
        return (EjsObj*) ((lhs->value < rhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_LE:
        return (EjsObj*) ((lhs->value <= rhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_GT:
        return (EjsObj*) ((lhs->value > rhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_GE:
        return (EjsObj*) ((lhs->value >= rhs->value) ? ejs->trueValue: ejs->falseValue);

    /*
     *  Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return (EjsObj*) ((lhs->value) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_ZERO:
        return (EjsObj*) ((lhs->value == 0) ? ejs->trueValue: ejs->falseValue);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return (EjsObj*) ejs->falseValue;

    case EJS_OP_NEG:
        return (EjsObj*) ejsCreateNumber(ejs, -lhs->value);

    case EJS_OP_LOGICAL_NOT:
        return (EjsObj*) ejsCreateBoolean(ejs, !fixed(lhs->value));

    case EJS_OP_NOT:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (~fixed(lhs->value)));


    /*
        Binary operations
     */
    case EJS_OP_ADD:
        return (EjsObj*) ejsCreateNumber(ejs, lhs->value + rhs->value);

    case EJS_OP_AND:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) & fixed(rhs->value)));

    case EJS_OP_DIV:
        return (EjsObj*) ejsCreateNumber(ejs, lhs->value / rhs->value);

    case EJS_OP_MUL:
        return (EjsObj*) ejsCreateNumber(ejs, lhs->value * rhs->value);

    case EJS_OP_OR:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) | fixed(rhs->value)));

    case EJS_OP_REM:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) % fixed(rhs->value)));

    case EJS_OP_SHL:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) << fixed(rhs->value)));

    case EJS_OP_SHR:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_SUB:
        return (EjsObj*) ejsCreateNumber(ejs, lhs->value - rhs->value);

    case EJS_OP_USHR:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) >> fixed(rhs->value)));

    case EJS_OP_XOR:
        return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) (fixed(lhs->value) ^ fixed(rhs->value)));

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %s", opcode, lhs->obj.type->qname.name);
        return 0;
    }
}


/*********************************** Methods **********************************/
/*
    Number constructor.
    function Number(value: Object = null)
 */
static EjsObj *numberConstructor(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    EjsNumber   *num;

    mprAssert(argc == 0 || argc == 1);

    if (argc == 1) {
        num = ejsToNumber(ejs, argv[0]);
        if (num) {
            np->value = num->value;
        }
    }
    return (EjsObj*) np;
}


/*
    Function to iterate and return each number in sequence.
    NOTE: this is not a method of Number. Rather, it is a callback function for Iterator.
 */
static EjsObj *nextNumber(Ejs *ejs, EjsIterator *ip, int argc, EjsObj **argv)
{
    EjsNumber   *np;

    np = (EjsNumber*) ip->target;
    if (!ejsIsNumber(np)) {
        ejsThrowReferenceError(ejs, "Wrong type");
        return 0;
    }

    if (ip->index < np->value) {
        return (EjsObj*) ejsCreateNumber(ejs, ip->index++);
    }
    ejsThrowStopIteration(ejs);
    return 0;
}


/*
    function integral(size: Number: 32): Number
 */
static EjsObj *integral(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
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
    return (EjsObj*) ejsCreateNumber(ejs, (MprNumber) result);
}


/*
    function get isFinite(): Boolean
 */
static EjsObj *isFinite(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    if (np->value == ejs->nanValue->value || 
            np->value == ejs->infinityValue->value || 
            np->value == ejs->negativeInfinityValue->value) {
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) ejs->trueValue;
}


/*
    function get isNaN(): Boolean
 */
static EjsObj *isNaN(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    return (EjsObj*) (mprIsNan(np->value) ? ejs->trueValue : ejs->falseValue);
}


/*
    function toExponential(fractionDigits: Number = 0): String
    Display with only one digit before the decimal point.
 */
static EjsObj *toExponential(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]): 0;
    result = mprDtoa(np, np->value, ndigits, MPR_DTOA_N_DIGITS, MPR_DTOA_EXPONENT_FORM);
    return (EjsObj*) ejsCreateStringAndFree(ejs, result);
}


/*
    function toFixed(fractionDigits: Number = 0): String

    Display the specified number of fractional digits
 */
static EjsObj *toFixed(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 0;
    result = mprDtoa(np, np->value, ndigits, MPR_DTOA_N_FRACTION_DIGITS, MPR_DTOA_FIXED_FORM);
    return (EjsObj*) ejsCreateStringAndFree(ejs, result);
}


/*
    function toPrecision(numDigits: Number = MAX_VALUE): String
    Display the specified number of total digits
 */
static EjsObj *toPrecision(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    char    *result;
    int     ndigits;
    
    ndigits = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 0;
    result = mprDtoa(np, np->value, ndigits, MPR_DTOA_N_DIGITS, 0);
    return (EjsObj*) ejsCreateStringAndFree(ejs, result);
}


/*
    Return the default iterator. This returns the index names.
    iterator native function get(): Iterator
 */
static EjsObj *getNumberIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateIterator(ejs, np, (EjsProc) nextNumber, 0, NULL);
}


/*
    Convert the number to a string.
    function toString(): String
 */
static EjsObj *numberToString(Ejs *ejs, EjsObj *vp, int argc, EjsObj **argv)
{
    return castNumber(ejs, (EjsNumber*) vp, ejs->stringType);
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
        return ejs->zeroValue;
    } else if (value == 1) {
        return ejs->oneValue;
    } else if (value == -1) {
        return ejs->minusOneValue;
    }
    vp = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    if (vp != 0) {
        vp->value = value;
    }
    ejsSetDebugName(vp, "number value");
    return vp;
}


void ejsCreateNumberType(Ejs *ejs)
{
    EjsType     *type;
    static int  zero = 0;

    type = ejs->numberType = ejsCreateNativeType(ejs, "ejs", "Number", ES_Number, sizeof(EjsNumber));
    type->immutable = 1;

    type->helpers.cast = (EjsCastHelper) castNumber;
    type->helpers.clone = (EjsCloneHelper) cloneNumber;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeNumberOperator;

    ejs->zeroValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->zeroValue->value = 0;
    ejs->oneValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->oneValue->value = 1;
    ejs->minusOneValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->minusOneValue->value = -1;

    ejs->infinityValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->infinityValue->value = 1.0 / zero;
    ejs->negativeInfinityValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->negativeInfinityValue->value = -1.0 / zero;
    ejs->nanValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->nanValue->value = 0.0 / zero;

    ejs->maxValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->maxValue->value = 1.7976931348623157e+308;
    ejs->minValue = (EjsNumber*) ejsCreate(ejs, ejs->numberType, 0);
    ejs->minValue->value = 5e-324;

    ejsSetDebugName(ejs->infinityValue, "Infinity");
    ejsSetDebugName(ejs->negativeInfinityValue, "NegativeInfinity");
    ejsSetDebugName(ejs->nanValue, "NaN");

    ejsSetDebugName(ejs->minusOneValue, "-1");
    ejsSetDebugName(ejs->oneValue, "1");
    ejsSetDebugName(ejs->zeroValue, "0");
    ejsSetDebugName(ejs->maxValue, "MaxValue");
    ejsSetDebugName(ejs->minValue, "MinValue");
}


void ejsConfigureNumberType(Ejs *ejs)
{
    EjsType         *type;
    EjsObj      *prototype;

    type = ejsGetTypeByName(ejs, "ejs", "Number");
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, (EjsProc) numberConstructor);
    ejsBindMethod(ejs, prototype, ES_Number_iterator_get, getNumberIterator);
    ejsBindMethod(ejs, prototype, ES_Number_iterator_getValues, getNumberIterator);
    ejsBindMethod(ejs, prototype, ES_Number_integral, (EjsProc) integral);
    ejsBindMethod(ejs, prototype, ES_Number_isFinite, (EjsProc) isFinite);
    ejsBindMethod(ejs, prototype, ES_Number_isNaN, (EjsProc) isNaN);
    ejsBindMethod(ejs, prototype, ES_Number_toExponential, (EjsProc) toExponential);
    ejsBindMethod(ejs, prototype, ES_Number_toFixed, (EjsProc) toFixed);
    ejsBindMethod(ejs, prototype, ES_Number_toPrecision, (EjsProc) toPrecision);
    ejsBindMethod(ejs, prototype, ES_Number_toString, numberToString);

    ejsSetProperty(ejs, type, ES_Number_MaxValue, ejs->maxValue);
    ejsSetProperty(ejs, type, ES_Number_MinValue, ejs->minValue);
    ejsSetProperty(ejs, type, ES_Number_NEGATIVE_INFINITY, ejs->negativeInfinityValue);
    ejsSetProperty(ejs, type, ES_Number_POSITIVE_INFINITY, ejs->infinityValue);
    ejsSetProperty(ejs, type, ES_Number_NaN, ejs->nanValue);

    ejsSetProperty(ejs, ejs->global, ES_NegativeInfinity, ejs->negativeInfinityValue);
    ejsSetProperty(ejs, ejs->global, ES_Infinity, ejs->infinityValue);
    ejsSetProperty(ejs, ejs->global, ES_NaN, ejs->nanValue);
    ejsSetProperty(ejs, ejs->global, ES_double, type);
#if ES_num
    ejsSetProperty(ejs, ejs->global, ES_num, type);
#endif
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
