/**
    ejsBoolean.c - Boolean native class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/******************************************************************************/
/*
    Cast the operand to a primitive type

    function cast(type: Type) : Object
 */
static EjsAny *castBooleanVar(Ejs *ejs, EjsBoolean *vp, EjsType *type)
{
    assure(ejsIs(ejs, vp, Boolean));

    switch (type->sid) {
    case S_Number:
        return ((vp->value) ? ESV(one): ESV(zero));

    case S_String:
        return ejsCreateStringFromAsc(ejs, (vp->value) ? "true" : "false");

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


/*
    Coerce operands for invokeOperator
 */
static EjsAny *coerceBooleanOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs)
{
    switch (opcode) {

    case EJS_OP_ADD:
        if (ejsIs(ejs, rhs, Void)) {
            return ESV(nan);
        } else if (ejsIs(ejs, rhs, Null) || ejsIs(ejs, rhs, Number) || ejsIs(ejs, rhs, Date)) {
            return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);
        } else {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        break;

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
    case EJS_OP_COMPARE_EQ: case EJS_OP_COMPARE_NE:
        if (ejsIs(ejs, rhs, String)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        return ejsInvokeOperator(ejs, ejsToNumber(ejs, lhs), opcode, rhs);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    /*
        Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_TRUE:
        return (((EjsBoolean*) lhs)->value ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
    case EJS_OP_COMPARE_FALSE:
        return (((EjsBoolean*) lhs)->value ? ESV(false) : ESV(true));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
}


/*
    Run an operator on the operands
 */
static EjsAny *invokeBooleanOperator(Ejs *ejs, EjsBoolean *lhs, int opcode, EjsBoolean *rhs)
{
    EjsAny      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if (!ejsIs(ejs, lhs, Boolean) || !ejsIs(ejs, rhs, Boolean)) {
            if ((result = coerceBooleanOperands(ejs, lhs, opcode, rhs)) != 0) {
                return result;
            }
        }
    }

    /*
        Types now match
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

    case EJS_OP_COMPARE_NOT_ZERO:
        return ((lhs->value) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_ZERO:
        return ((lhs->value == 0) ? ESV(true): ESV(false));

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NULL:
        return ESV(false);

    case EJS_OP_COMPARE_FALSE:
        return ((lhs->value) ? ESV(false): ESV(true));

    case EJS_OP_COMPARE_TRUE:
        return ((lhs->value) ? ESV(true): ESV(false));

    /*
        Unary operators
     */
    case EJS_OP_NEG:
        return ejsCreateNumber(ejs, - lhs->value);

    case EJS_OP_LOGICAL_NOT:
        return ejsCreateBoolean(ejs, !lhs->value);

    case EJS_OP_NOT:
        return ejsCreateBoolean(ejs, ~lhs->value);

    /*
        Binary operations
     */
    case EJS_OP_ADD:
        return ejsCreateBoolean(ejs, lhs->value + rhs->value);

    case EJS_OP_AND:
        return ejsCreateBoolean(ejs, lhs->value & rhs->value);

    case EJS_OP_DIV:
        return ejsCreateBoolean(ejs, lhs->value / rhs->value);

    case EJS_OP_MUL:
        return ejsCreateBoolean(ejs, lhs->value * rhs->value);

    case EJS_OP_OR:
        return ejsCreateBoolean(ejs, lhs->value | rhs->value);

    case EJS_OP_REM:
        return ejsCreateBoolean(ejs, lhs->value % rhs->value);

    case EJS_OP_SUB:
        return ejsCreateBoolean(ejs, lhs->value - rhs->value);

    case EJS_OP_USHR:
        return ejsCreateBoolean(ejs, lhs->value >> rhs->value);

    case EJS_OP_XOR:
        return ejsCreateBoolean(ejs, lhs->value ^ rhs->value);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
}


/*********************************** Methods **********************************/
/*
    Boolean constructor.

        function Boolean(value: Boolean = null)

    If the value is omitted or 0, -1, NaN, false, null, undefined or the empty string, then set the boolean value to
    to false.
 */
static EjsBoolean *booleanConstructor(Ejs *ejs, EjsBoolean *bp, int argc, EjsObj **argv)
{
    assure(argc == 0 || argc == 1);

    if (argc >= 1) {
        /* Change the bp value */
        bp->value = ejsToBoolean(ejs, argv[0])->value;
    }
    return bp;
}


/*********************************** Factory **********************************/

PUBLIC void ejsCreateBooleanType(Ejs *ejs)
{
    EjsType     *type;
    EjsBoolean  *vp;

    type = ejsCreateCoreType(ejs, N("ejs", "Boolean"), sizeof(EjsBoolean), S_Boolean, ES_Boolean_NUM_CLASS_PROP, 
        0, EJS_TYPE_OBJ | EJS_TYPE_IMMUTABLE_INSTANCES);
    type->helpers.cast = (EjsCastHelper) castBooleanVar;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeBooleanOperator;

    /*
        Pre-create the only two valid instances for boolean
     */
    vp = ejsCreateObj(ejs, type, 0);
    vp->value = 1;
    ejsAddImmutable(ejs, S_true, EN("true"), vp);

    vp = ejsCreateObj(ejs, type, 0);
    vp->value = 0;
    ejsAddImmutable(ejs, S_false, EN("false"), vp);
}


PUBLIC void ejsConfigureBooleanType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Boolean"))) == 0) {
        return;
    }
    ejsBindConstructor(ejs, type, booleanConstructor);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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
