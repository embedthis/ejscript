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
        return S(false);

    case S_Number:
        return S(zero);

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
        return ejsInvokeOperator(ejs, S(zero), opcode, rhs);

    /*
        Comparision
     */
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        if (ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, S(zero), opcode, rhs);
        } else if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_COMPARE_NE:
        if (ejsIs(ejs, rhs, Void)) {
            return S(false);
        }
        return S(true);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return S(true);

    case EJS_OP_COMPARE_EQ:
        if (ejsIs(ejs, rhs, Void)) {
            return S(true);
        }
        return S(false);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return S(false);

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return S(true);

    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return S(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return S(undefined);
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
        return S(true);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return S(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return S(one);

    /*
        Binary operators. Reinvoke with left = zero
     */
    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, S(zero), opcode, rhs);

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
    return ejsCreateIterator(ejs, np, NULL, 0, NULL);
}


static EjsObj *getNullProperty(Ejs *ejs, EjsNull *unused, int slotNum)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
    return 0;
}


/*********************************** Factory **********************************/
/*
    We dont actually allocate any nulls. We just reuse the singleton instance.
 */

EjsNull *ejsCreateNull(Ejs *ejs)
{
    return S(null);
}


void ejsInitNullType(Ejs *ejs, EjsType *type)
{
#if UNUSED
    EjsType     *type;

    type = ejsCreateNativeType(ejs, N("ejs", "Null"), sizeof(EjsNull), S_Null, ES_Null_NUM_CLASS_PROP, 
        NULL, EJS_OBJ_HELPERS);
#endif

    type->helpers.cast             = (EjsCastHelper) castNull;
    type->helpers.getProperty      = (EjsGetPropertyHelper) getNullProperty;
    type->helpers.invokeOperator   = (EjsInvokeOperatorHelper) invokeNullOperator;
#if UNUSED
    ejsSetSpecial(ejs, ES_null, ejsCreateObj(ejs, type, 0));
    mprSetName(S(null), "null");
#endif
}


void ejsConfigureNullType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = S(Null);
    prototype = type->prototype;

    ejsSetProperty(ejs, ejs->global, ES_null, S(null));
    ejsBindMethod(ejs, prototype, ES_Null_iterator_get, getNullIterator);
    ejsBindMethod(ejs, prototype, ES_Null_iterator_getValues, getNullIterator);
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
