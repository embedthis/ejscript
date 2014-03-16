/**
    ejsNumber.c - Number type class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************** Defines ***********************************/
/*
   Mode values for nota
 */
#define DTOA_ALL_DIGITS         0       /**< Return all digits */
#define DTOA_N_DIGITS           2       /**< Return total N digits */
#define DTOA_N_FRACTION_DIGITS  3       /**< Return total fraction digits */

/*
    Flags for mprDtoa
 */
#define DTOA_EXPONENT_FORM      0x10    /**< Result in exponent form (N.NNNNe+NN) */
#define DTOA_FIXED_FORM         0x20    /**< Emit in fixed form (NNNN.MMMM)*/

#if UNUSED
/**
    Convert a double to ascii
    @param value Value to convert
    @param ndigits Number of digits to render
    @param mode Modes are:
         0   Shortest string,
         1   Like 0, but with Steele & White stopping rule,
         2   Return ndigits of result,
         3   Number of digits applies after the decimal point.
    @param flags Format flags
 */
extern char *mprDtoa(double value, int ndigits, int mode, int flags);
#endif

extern char *dtoa(double d, int mode, int ndigits, int* decpt, int* sign, char** rve);
extern void freedtoa(char* ptr);

/**************************** Forward Declarations ****************************/

#define fixed(n) ((int64) (floor(n)))
static char *ntoa(double value, int ndigits, int mode, int flags);

/******************************************************************************/
/*
    Cast the operand to the specified type
 */
static EjsAny *castNumber(Ejs *ejs, EjsNumber *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ((vp->value && !ejsIsNan(vp->value)) ? ESV(true) : ESV(false));

    case S_String:
        //  OPT. ntoa does a clone
        return ejsCreateStringFromAsc(ejs, ntoa(vp->value, 0, 0, 0));

    case S_Number:
        return vp;
            
    default:
        ejsThrowTypeError(ejs, "Cannot cast to this type");
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
            return ESV(nan);
        } else if (ejsIs(ejs, rhs, Null)) {
            return lhs;
        } else if (ejsIs(ejs, rhs, Boolean) || ejsIs(ejs, rhs, Date)) {
            return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));
        }
        return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opcode == EJS_OP_COMPARE_EQ) ? ESV(false): ESV(true));
        } else if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        } else if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, lhs, opcode, ejsToNumber(ejs, rhs));

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (((EjsNumber*) lhs)->value ? ESV(true) : ESV(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsNumber*) lhs)->value ? ESV(false): ESV(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
}


static EjsAny *invokeNumberOperator(Ejs *ejs, EjsNumber *lhs, int opcode, EjsNumber *rhs)
{
    EjsObj      *result;

    assert(lhs);
    
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
        return ((lhs->value == rhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
        return ((lhs->value != rhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_LT:
        return ((lhs->value < rhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_LE:
        return ((lhs->value <= rhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_GT:
        return ((lhs->value > rhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_GE:
        return ((lhs->value >= rhs->value) ? ESV(true): ESV(false));

    /*
     *  Unary operators
     */
    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->value == 0) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
        return ESV(false);

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

    assert(argc == 0 || argc == 1);

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
    if (np->value == ((EjsNumber*) ESV(nan))->value || np->value == ((EjsNumber*) ESV(infinity))->value || 
            np->value == ((EjsNumber*) ESV(negativeInfinity))->value) {
        return ESV(false);
    }
    return ESV(true);
}


/*
    function get isNaN(): Boolean
 */
static EjsBoolean *isNaN(Ejs *ejs, EjsNumber *np, int argc, EjsObj **argv)
{
    return (mprIsNan(np->value) ? ESV(true) : ESV(false));
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
    result = ntoa(np->value, ndigits, DTOA_N_DIGITS, DTOA_EXPONENT_FORM);
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
    result = ntoa(np->value, ndigits, DTOA_N_FRACTION_DIGITS, DTOA_FIXED_FORM);
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
    result = ntoa(np->value, ndigits, DTOA_N_DIGITS, 0);
    return ejsCreateStringFromAsc(ejs, result);
}


/*
    Return the default iterator. This returns the index names.

    iterator native function get(): Iterator
 */
static EjsIterator *getNumberIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, np, -1, nextNumber, 0, NULL);
}


/*
    Convert the number to a string.
    function toString(): String
 */
static EjsObj *numberToString(Ejs *ejs, EjsNumber *vp, int argc, EjsObj **argv)
{
    return castNumber(ejs, vp, ESV(String));
}


/*********************************** Support **********************************/

#ifndef ejsIsNan
PUBLIC int ejsIsNan(double f)
{
#if ME_WIN_LIKE
    return _isnan(f);
#elif VXWORKS
    return isnan(f);
#else
    return (f == FP_NAN);
#endif
}
#endif


PUBLIC bool ejsIsInfinite(MprNumber f)
{
#if ME_WIN_LIKE
    return !_finite(f);
#elif VXWORKS
    return isinf(f);
#else
    return (f == FP_INFINITE);
#endif
}


PUBLIC void ejsLockDtoa(int n)
{
    EjsService  *es;
    
    es = MPR->ejsService;
    mprSpinLock(es->dtoaSpin[n]);
}


PUBLIC void ejsUnlockDtoa(int n)
{
    EjsService  *es;
    
    es = MPR->ejsService;
    mprSpinUnlock(es->dtoaSpin[n]);
}


/*
    Convert a double to ascii. Caller must free the result. This uses the JavaScript ECMA-262 spec for formatting rules.
 */
static char *ntoa(double value, int ndigits, int mode, int flags)
{
    MprBuf  *buf;
    char    *intermediate, *ip;
    int     period, sign, len, exponentForm, fixedForm, exponent, count, totalDigits, npad;

    buf = mprCreateBuf(64, -1);
    intermediate = 0;
    exponentForm = 0;
    fixedForm = 0;

    if (mprIsNan(value)) {
        mprPutStringToBuf(buf, "NaN");

    } else if (mprIsInfinite(value)) {
        if (value < 0) {
            mprPutStringToBuf(buf, "-Infinity");
        } else {
            mprPutStringToBuf(buf, "Infinity");
        }
    } else if (value == 0) {
        mprPutCharToBuf(buf, '0');

    } else {
        if (ndigits <= 0) {
            if (!(flags & DTOA_FIXED_FORM)) {
                mode = DTOA_ALL_DIGITS;
            }
            ndigits = 0;

        } else if (mode == DTOA_ALL_DIGITS) {
            mode = DTOA_N_DIGITS;
        }
        if (flags & DTOA_EXPONENT_FORM) {
            exponentForm = 1;
            if (ndigits > 0) {
                ndigits++;
            } else {
                ndigits = 0;
                mode = DTOA_ALL_DIGITS;
            }
        } else if (flags & DTOA_FIXED_FORM) {
            fixedForm = 1;
        }

        /*
            Convert to an intermediate string representation. Period is the offset of the decimal point. NOTE: the
            intermediate representation may have less digits than period.
            Note: ndigits < 0 seems to trim N digits from the end with rounding.
         */
        ip = intermediate = dtoa(value, mode, ndigits, &period, &sign, NULL);
        len = (int) slen(intermediate);
        exponent = period - 1;

        if (mode == DTOA_ALL_DIGITS && ndigits == 0) {
            ndigits = len;
        }
        if (!fixedForm) {
            if (period <= -6 || period > 21) {
                exponentForm = 1;
            }
        }
        if (sign) {
            mprPutCharToBuf(buf, '-');
        }
        if (exponentForm) {
            mprPutCharToBuf(buf, ip[0] ? ip[0] : '0');
            if (len > 1) {
                mprPutCharToBuf(buf, '.');
                mprPutSubStringToBuf(buf, &ip[1], (ndigits == 0) ? len - 1: ndigits);
            }
            mprPutCharToBuf(buf, 'e');
            mprPutCharToBuf(buf, (period < 0) ? '-' : '+');
            mprPutToBuf(buf, "%d", (exponent < 0) ? -exponent: exponent);

        } else {
            if (mode == DTOA_N_FRACTION_DIGITS) {
                /* Count of digits */
                if (period <= 0) {
                    /* Leading fractional zeros required */
                    mprPutStringToBuf(buf, "0.");
                    mprPutPadToBuf(buf, '0', -period);
                    mprPutStringToBuf(buf, ip);
                    npad = ndigits - len + period;
                    if (npad > 0) {
                        mprPutPadToBuf(buf, '0', npad);
                    }

                } else {
                    count = min(len, period);
                    /* Leading integral digits */
                    mprPutSubStringToBuf(buf, ip, count);
                    /* Trailing zero pad */
                    if (period > len) {
                        mprPutPadToBuf(buf, '0', period - len);
                    }
                    totalDigits = count + ndigits;
                    if (period < totalDigits) {
                        count = totalDigits + sign - (int) mprGetBufLength(buf);
                        mprPutCharToBuf(buf, '.');
                        mprPutSubStringToBuf(buf, &ip[period], count);
                        mprPutPadToBuf(buf, '0', count - slen(&ip[period]));
                    }
                }

            } else if (len <= period && period <= 21) {
                /* data shorter than period */
                mprPutStringToBuf(buf, ip);
                mprPutPadToBuf(buf, '0', period - len);

            } else if (0 < period && period <= 21) {
                /* Period shorter than data */
                mprPutSubStringToBuf(buf, ip, period);
                mprPutCharToBuf(buf, '.');
                mprPutStringToBuf(buf, &ip[period]);

            } else if (-6 < period && period <= 0) {
                /* Small negative exponent */
                mprPutStringToBuf(buf, "0.");
                mprPutPadToBuf(buf, '0', -period);
                mprPutStringToBuf(buf, ip);

            } else {
                assert(0);
            }
        }
    }
    mprAddNullToBuf(buf);
    if (intermediate) {
        freedtoa(intermediate);
    }
    return sclone(mprGetBufStart(buf));
}


/*********************************** Factory **********************************/
/*
    Create an initialized number
 */

PUBLIC EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value)
{
    EjsNumber   *vp;

    if (value == 0) {
        return ESV(zero);
    } else if (value == 1) {
        return ESV(one);
    } else if (value == -1) {
        return ESV(minusOne);
    }
    if ((vp = ejsCreateObj(ejs, ESV(Number), 0)) != 0) {
        vp->value = value;
    }
    return vp;
}


PUBLIC void ejsCreateNumberType(Ejs *ejs)
{
    EjsNumber   *np;
    EjsType     *type;
    static int  zero = 0;

    type = ejsCreateCoreType(ejs, N("ejs", "Number"), sizeof(EjsNumber), S_Number, ES_Number_NUM_CLASS_PROP, 
        NULL, EJS_TYPE_OBJ | EJS_TYPE_IMMUTABLE_INSTANCES);

    type->helpers.cast = (EjsCastHelper) castNumber;
    type->helpers.clone = (EjsCloneHelper) cloneNumber;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeNumberOperator;

    np = ejsCreateObj(ejs, type, 0);
    np->value = 0;
    ejsAddImmutable(ejs, S_zero, EN("zero"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1;
    ejsAddImmutable(ejs, S_one, EN("one"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = -1;
    ejsAddImmutable(ejs, S_minusOne, EN("minusOne"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1.0 / zero;
    ejsAddImmutable(ejs, S_infinity, EN("Infinity"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = -1.0 / zero;
    ejsAddImmutable(ejs, S_negativeInfinity, EN("NegativeInfinity"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 0.0 / zero;
    ejsAddImmutable(ejs, S_nan, EN("NaN"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 1.7976931348623157e+308;
    ejsAddImmutable(ejs, S_max, EN("max"), np);

    np = ejsCreateObj(ejs, type, 0);
    np->value = 5e-324;
    ejsAddImmutable(ejs, S_min, EN("min"), np);
}


PUBLIC void ejsConfigureNumberType(Ejs *ejs)
{
    EjsType    *type;
    EjsPot     *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Number"))) == 0) {
        return;
    }
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

    ejsSetProperty(ejs, type, ES_Number_MaxValue, ESV(max));
    ejsSetProperty(ejs, type, ES_Number_MinValue, ESV(min));
    ejsSetProperty(ejs, type, ES_Number_NEGATIVE_INFINITY, ESV(negativeInfinity));
    ejsSetProperty(ejs, type, ES_Number_POSITIVE_INFINITY, ESV(infinity));
    ejsSetProperty(ejs, type, ES_Number_NaN, ESV(nan));
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
