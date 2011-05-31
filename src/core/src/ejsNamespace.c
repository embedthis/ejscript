/**
    ejsNamespace.c - Ejscript Namespace class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/
/*
    Cast the operand to the specified type
 */

static EjsAny *castNamespace(Ejs *ejs, EjsNamespace *vp, EjsType *type)
{
    switch (type->sid) {
    case S_Boolean:
        return ejsCreateBoolean(ejs, 1);

    case S_String:
        return ejsCreateStringFromAsc(ejs, "[object Namespace]");

    default:
        ejsThrowTypeError(ejs, "Can't cast to this type");
        return 0;
    }
}


static EjsAny *invokeNamespaceOperator(Ejs *ejs, EjsNamespace *lhs, int opCode, EjsNamespace *rhs)
{
    bool        boolResult;

    switch (opCode) {
    case EJS_OP_COMPARE_EQ:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opCode == EJS_OP_COMPARE_EQ) ? S(false): S(true));
        }
        boolResult = ejsCompareString(ejs, lhs->value, rhs->value) == 0;
        break;

    case EJS_OP_COMPARE_STRICTLY_EQ:
        boolResult = lhs == rhs;
        break;

    case EJS_OP_COMPARE_NE:
        if (!ejsIsDefined(ejs, rhs)) {
            return ((opCode == EJS_OP_COMPARE_EQ) ? S(false): S(true));
        }
        boolResult = !(ejsCompareString(ejs, lhs->value, rhs->value) == 0);
        break;

    case EJS_OP_COMPARE_STRICTLY_NE:
        boolResult = !(lhs == rhs);
        break;

    default:
        ejsThrowTypeError(ejs, "Operation is not valid on this type");
        return 0;
    }
    return ejsCreateBoolean(ejs, boolResult);
}


/*
    Define a reserved namespace in a block.
 */
EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *spaceName)
{
    EjsNamespace    *namespace;

    namespace = ejsCreateReservedNamespace(ejs, typeName, ejsCreateStringFromAsc(ejs, spaceName));
    if (namespace) {
        if (ejsAddNamespaceToBlock(ejs, block, namespace) < 0) {
            return 0;
        }
    }
    return namespace;
}


/*
    Format a reserved namespace to create a unique namespace. "internal, public, private, protected"

    Namespaces are formatted as strings using the following format, where type is optional. Types may be qualified.
        [type,space]

    Example:
        [debug::Shape,public] where Shape was declared as "debug class Shape"
 */
EjsString *ejsFormatReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName)
{
    EjsString   *namespace;

    if (typeName) {
        if (typeName->space && typeName->space == S(public)) {
            namespace = ejsSprintf(ejs, "[%N,%@]", *typeName, spaceName);
        } else {
            namespace = ejsSprintf(ejs, "[%@,%@]", typeName->name, spaceName);
        }
    } else {
        namespace = ejsSprintf(ejs, "[%@]", spaceName);
    }
    return namespace;
}


/*********************************** Factory **********************************/
/*
    Create a namespace with the given Uri as its definition qualifying value.
 */
EjsNamespace *ejsCreateNamespace(Ejs *ejs, EjsString *name)
{
    EjsNamespace    *np;

    if ((np = ejsCreateObj(ejs, S(Namespace), 0)) == 0) {
        return 0;
    }
    np->value = name;
    mprSetName(np, "namespace");
    return np;
}


/*
    Create a reserved namespace. Format the package, type and space names to create a unique namespace.
 */
EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName)
{
    EjsString       *formattedName;

    mprAssert(spaceName);

    if (typeName) {
        formattedName = ejsFormatReservedNamespace(ejs, typeName, spaceName);
    } else {
        formattedName = spaceName;
    }
    return ejsCreateNamespace(ejs, formattedName);
}


static void manageNamespace(EjsNamespace *ns, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ns->value);
    }
}


void ejsCreateNamespaceType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", "Namespace"), sizeof(EjsNamespace), S_Namespace, ES_Namespace_NUM_CLASS_PROP,
        manageNamespace, EJS_TYPE_OBJ | EJS_TYPE_IMMUTABLE_INSTANCES);
    type->helpers.cast = (EjsCastHelper) castNamespace;
    type->helpers.invokeOperator = (EjsInvokeOperatorHelper) invokeNamespaceOperator;
}


void ejsConfigureNamespaceType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", "Namespace"))) == 0) {
        return;
    }
    ejsSetProperty(ejs, ejs->global, ES_iterator, S(iteratorSpace));
    ejsSetProperty(ejs, ejs->global, ES_public, S(publicSpace));
    ejsSetProperty(ejs, ejs->global, ES_ejs, S(ejsSpace));
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
