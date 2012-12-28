/**
    ejsVoid.c - Ejscript Void class (aka undefined)

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/********************************* Void Helpers *******************************/
/*
    Cast the void operand to a primitive type
 */

static EjsAny *castVoid(Ejs *ejs, EjsVoid *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ESV(false);

    case S_Number:
        return ESV(nan);

    case S_Object:
        return vp;

    case S_String:
        return ejsCreateStringFromAsc(ejs, "undefined");

    case S_Uri:
        return ejsCreateUriFromAsc(ejs, "undefined");
            
    default:
        ejsThrowTypeError(ejs, "Cannot cast to this type");
        return 0;
    }
}



static EjsAny *coerceVoidOperands(Ejs *ejs, EjsVoid *lhs, int opcode, EjsVoid *rhs)
{
    switch (opcode) {

    case EJS_OP_ADD:
        if (!ejsIs(ejs, rhs, Number)) {
            return ejsInvokeOperator(ejs, ejsToString(ejs, lhs), opcode, rhs);
        }
        /* Fall through */

    case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ejsInvokeOperator(ejs, ESV(nan), opcode, rhs);

    /*
     *  Comparision
     */
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        return ESV(false);

    case EJS_OP_COMPARE_NE:
        if (ejsIs(ejs, rhs, Null)) {
            return ESV(false);
        }
        return ESV(true);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return ESV(true);

    case EJS_OP_COMPARE_EQ:
        if (ejsIs(ejs, rhs, Null)) {
            return ESV(true);
        }
        return ESV(false);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return ESV(false);

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return ESV(true);

    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return ESV(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return ESV(undefined);
    }
    return 0;
}



static EjsObj *invokeVoidOperator(Ejs *ejs, EjsVoid *lhs, int opcode, EjsVoid *rhs)
{
    EjsObj      *result;

    if (rhs == 0 || TYPE(lhs) != TYPE(rhs)) {
        if ((result = coerceVoidOperands(ejs, lhs, opcode, rhs)) != 0) {
            return result;
        }
    }

    /*
     *  Types match, left and right types are both "undefined"
     */
    switch (opcode) {

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
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return ESV(nan);

    /*
     *  Binary operators
     */
    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return ESV(nan);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    assure(0);
}


/*
    iterator native function get(): Iterator
 */
static EjsIterator *getVoidIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, np, -1, NULL, 0, NULL);
}


static EjsObj *getVoidProperty(Ejs *ejs, EjsVoid *unused, int slotNum)
{
    ejsThrowReferenceError(ejs, "Object reference is null");
    return 0;
}


/*********************************** Factory **********************************/
/*
    We don't actually create any instances. We just use a reference to the undefined singleton instance.
 */
PUBLIC EjsVoid *ejsCreateUndefined(Ejs *ejs)
{
    return (EjsVoid*) ESV(undefined);
}


PUBLIC void ejsCreateVoidType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "Void"), sizeof(EjsVoid), S_Void, ES_Void_NUM_CLASS_PROP, NULL, 
        EJS_TYPE_OBJ);

    type->helpers.cast             = (EjsCastHelper) castVoid;
    type->helpers.invokeOperator   = (EjsInvokeOperatorHelper) invokeVoidOperator;
    type->helpers.getProperty      = (EjsGetPropertyHelper) getVoidProperty;

    ejsAddImmutable(ejs, ES_undefined, EN("undefined"), ejsCreateObj(ejs, type, 0));
    mprSetName(ESV(undefined), "undefined");
}


PUBLIC void ejsConfigureVoidType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Void"))) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_Void_iterator_get, getVoidIterator);
    ejsBindMethod(ejs, prototype, ES_Void_iterator_getValues, getVoidIterator);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
