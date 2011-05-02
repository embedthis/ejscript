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
        return S(false);

    case S_Number:
        return S(nan);

    case S_Object:
        return vp;

    case S_String:
        return ejsCreateStringFromAsc(ejs, "undefined");

    case S_Uri:
        return ejsCreateUriFromMulti(ejs, "undefined");
            
    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
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
        return ejsInvokeOperator(ejs, S(nan), opcode, rhs);

    /*
     *  Comparision
     */
    case EJS_OP_COMPARE_LE: case EJS_OP_COMPARE_LT:
    case EJS_OP_COMPARE_GE: case EJS_OP_COMPARE_GT:
        return S(false);

    case EJS_OP_COMPARE_NE:
        if (ejsIs(ejs, rhs, Null)) {
            return S(false);
        }
        return S(true);

    case EJS_OP_COMPARE_STRICTLY_NE:
        return S(true);

    case EJS_OP_COMPARE_EQ:
        if (ejsIs(ejs, rhs, Null)) {
            return S(true);
        }
        return S(false);

    case EJS_OP_COMPARE_STRICTLY_EQ:
        return S(false);

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return 0;

    case EJS_OP_COMPARE_UNDEFINED:
    case EJS_OP_COMPARE_NOT_ZERO:
    case EJS_OP_COMPARE_NULL:
        return S(true);

    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return S(false);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not valid for type %@", opcode, TYPE(lhs)->qname.name);
        return S(undefined);
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
        return S(true);

    case EJS_OP_COMPARE_NE: case EJS_OP_COMPARE_STRICTLY_NE:
    case EJS_OP_COMPARE_LT: case EJS_OP_COMPARE_GT:
    case EJS_OP_COMPARE_FALSE:
    case EJS_OP_COMPARE_TRUE:
    case EJS_OP_COMPARE_ZERO:
        return S(false);

    /*
     *  Unary operators
     */
    case EJS_OP_LOGICAL_NOT: case EJS_OP_NOT: case EJS_OP_NEG:
        return S(nan);

    /*
     *  Binary operators
     */
    case EJS_OP_ADD: case EJS_OP_AND: case EJS_OP_DIV: case EJS_OP_MUL: case EJS_OP_OR: case EJS_OP_REM:
    case EJS_OP_SHL: case EJS_OP_SHR: case EJS_OP_SUB: case EJS_OP_USHR: case EJS_OP_XOR:
        return S(nan);

    default:
        ejsThrowTypeError(ejs, "Opcode %d not implemented for type %@", opcode, TYPE(lhs)->qname.name);
        return 0;
    }
    mprAssert(0);
}


/*
    iterator native function get(): Iterator
 */
static EjsIterator *getVoidIterator(Ejs *ejs, EjsObj *np, int argc, EjsObj **argv)
{
    return ejsCreateIterator(ejs, np, NULL, 0, NULL);
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
EjsVoid *ejsCreateUndefined(Ejs *ejs)
{
    return (EjsVoid*) S(undefined);
}


void ejsCreateVoidType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateNativeType(ejs, N("ejs", "Void"), sizeof(EjsVoid), S_Void, ES_Void_NUM_CLASS_PROP, NULL, 
        EJS_OBJ_HELPERS);

    type->helpers.cast             = (EjsCastHelper) castVoid;
    type->helpers.invokeOperator   = (EjsInvokeOperatorHelper) invokeVoidOperator;
    type->helpers.getProperty      = (EjsGetPropertyHelper) getVoidProperty;

    ejsSetSpecial(ejs, S_undefined, ejsCreateObj(ejs, type, 0));
    mprSetName(S(undefined), "undefined");
}


void ejsConfigureVoidType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ST(Void);
    prototype = type->prototype;

    ejsSetProperty(ejs, ejs->global, ES_void, type);
    ejsSetProperty(ejs, ejs->global, ES_undefined, S(undefined));
    ejsBindMethod(ejs, prototype, ES_Void_iterator_get, getVoidIterator);
    ejsBindMethod(ejs, prototype, ES_Void_iterator_getValues, getVoidIterator);
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
