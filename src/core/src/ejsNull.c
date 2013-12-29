/**
    ejsNull.c - Ejscript Null class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/
/*
    Cast the null operand to a primitive type
 */

static EjsAny *castNull(Ejs *ejs, EjsObj *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ESV(false);

    case S_Number:
        return ESV(zero);

    case S_Object:
    default:
        /*
            Cast null to anything else results in a null
         */
        return vp;

    case S_String:
        return ejsCreateStringFromAsc(ejs, "null");
    }
}


static EjsAny *coerceNullOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    switch (opcode) {

    case EJS_OP_ADD:
        if (!ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        /* Fall through */

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ESV(zero), opcode, rhs);

    /*
        Comparision
     */
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ESV(zero), opcode, rhs);
        } else if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_COMPARE_NE:
        if (ejsIs(ejs, rhs, Void)) {
            return ESV(false);
        }
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_EQ:
        if (ejsIs(ejs, rhs, Void)) {
            return ESV(true);
        }
        return ESV(false);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return ESV(true);

    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
    return 0;
}


static EjsAny *invokeNullOperator(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coerceNullOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    /*
        Types now match. Both left and right types are both "null"
     */
    switch (opcode) {

    /*
        NOTE: strict eq is the same as eq
     */
    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_STRICTLY_EQ:
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_GE:
    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return ESV(true);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return ESV(one);

    /*
        Binary operators. Reinvoke with left = zero
     */
    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ESV(zero), opcode, rhs);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
}


/*
    iterator native function get(): Iterator
 */
static EjsIterator *getNullIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, np, -1, NULL, 0, NULL);
}


static EjsObj *getNullProperty(Ejs *ejs, EjsNull *unused, int slotNum)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
    return 0;
}


/*********************************** Factory **********************************/
/*
    We dont actually allocate any nulls. We just reuse the singleton instance.
    OPT - macro
 */
PUBLIC EjsNull *ejsCreateNull(Ejs *ejs)
{
    return ESV(null);
}


PUBLIC void ejsInitNullType(Ejs *ejs, EjsType *type)
{
    type->helpers.cast             = (EjsCastHelper) castNull;
    type->helpers.getProperty      = (EjsGetPropertyHelper) getNullProperty;
    type->helpers.invokeOperator   = (EjsInvokeOperatorHelper) invokeNullOperator;
}


PUBLIC void ejsConfigureNullType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Null"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_Null_iterator_get, getNullIterator);
    ejsBindMethod(ejs, prototype, ES_Null_iterator_getValues, getNullIterator);
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
