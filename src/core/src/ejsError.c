/**
    ejsError.c - Error Exception class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/
/*
    Cast the operand to the specified type

    function cast(type: Type) : Object
 */

static EjsObj *castError(Ejs *ejs, EjsError *vp, EjsType *type)
{
    EjsObj      *sp;
    char        *buf;

    switch (type->id) {

    case ES_Boolean:
        return (EjsObj*) ejsCreateBoolean(ejs, 1);

    case ES_String:
        if ((buf = mprAsprintf(ejs, -1,
                "%s Exception: %s\nStack:\n%s\n", vp->obj.type->qname.name, vp->message, vp->stack)) == NULL) {
            ejsThrowMemoryError(ejs);
        }
        sp = (EjsObj*) ejsCreateString(ejs, buf);
        mprFree(buf);
        return sp;

    default:
        ejsThrowTypeError(ejs, "Unknown type");
        return 0;
    }
}


/*
    Get a property.
 */
static EjsObj *getErrorProperty(Ejs *ejs, EjsError *error, int slotNum)
{
    switch (slotNum) {
    case ES_Error_stack:
        return (EjsObj*) ejsCreateString(ejs, error->stack);

    case ES_Error_message:
        return (EjsObj*) ejsCreateString(ejs, error->message);
    }
    return (ejs->objectType->helpers->getProperty)(ejs, (EjsObj*) error, slotNum);
}


/*
    Lookup a property.
 */
static int lookupErrorProperty(Ejs *ejs, EjsError *error, EjsName *qname)
{
    if (strcmp(qname->name, "message") == 0) {
        return ES_Error_message;
    }
    if (strcmp(qname->name, "stack") == 0) {
        return ES_Error_stack;
    }
    return -1;
}


/*********************************** Methods **********************************/
/*
    Error Constructor and constructor for all the core error classes.

    public function Error(message: String = null)
 */
static EjsObj *errorConstructor(Ejs *ejs, EjsError *error, int argc,  EjsObj **argv)
{
    mprFree(error->message);
    if (argc == 0) {
        error->message = mprStrdup(error, "");
    } else {
        error->message = mprStrdup(error, ejsGetString(ejs, argv[0]));
    }
    mprFree(error->stack);
    ejsFormatStack(ejs, error);
    return (EjsObj*) error;
}


static EjsObj *getCode(Ejs *ejs, EjsError *vp, int argc,  EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, vp->code);
}


static EjsObj *setCode(Ejs *ejs, EjsError *vp, int argc,  EjsObj **argv)
{
    vp->code = ejsGetInt(ejs, argv[0]);
    return 0;
}


/************************************ Factory *********************************/

static EjsType *defineType(Ejs *ejs, cchar *name, int id)
{
    EjsType     *type;

    type = ejsCreateNativeType(ejs, "ejs", name, id, sizeof(EjsError));
    type->block.nobind = 1;
    type->helpers->cast = (EjsCastHelper) castError;
    type->helpers->getProperty = (EjsGetPropertyHelper) getErrorProperty;
    type->helpers->lookupProperty = (EjsLookupPropertyHelper) lookupErrorProperty;
    return type;
}


void ejsCreateErrorType(Ejs *ejs)
{
    ejs->errorType = defineType(ejs, "Error", ES_Error);
    defineType(ejs, "ArgError", ES_ArgError);
    defineType(ejs, "ArithmeticError", ES_ArithmeticError);
    defineType(ejs, "AssertError", ES_AssertError);
    defineType(ejs, "InstructionError", ES_InstructionError);
    defineType(ejs, "IOError", ES_IOError);
    defineType(ejs, "InternalError", ES_InternalError);
    defineType(ejs, "MemoryError", ES_MemoryError);
    defineType(ejs, "OutOfBoundsError", ES_OutOfBoundsError);
    defineType(ejs, "ReferenceError", ES_ReferenceError);
    defineType(ejs, "ResourceError", ES_ResourceError);
    defineType(ejs, "SecurityError", ES_SecurityError);
    defineType(ejs, "StateError", ES_StateError);
    defineType(ejs, "SyntaxError", ES_SyntaxError);
    defineType(ejs, "TypeError", ES_TypeError);
    defineType(ejs, "URIError", ES_URIError);

}


static void configureType(Ejs *ejs, cchar *name)
{
    EjsType     *type;

    type = ejsGetTypeByName(ejs, "ejs", name);
    ejsBindMethod(ejs, type, type->numPrototypeInherited, (EjsProc) errorConstructor);
}


void ejsConfigureErrorType(Ejs *ejs)
{
    configureType(ejs, "Error");
    configureType(ejs, "ArgError");
    configureType(ejs, "ArithmeticError");
    configureType(ejs, "AssertError");
    configureType(ejs, "InstructionError");
    configureType(ejs, "IOError");
    configureType(ejs, "InternalError");
    configureType(ejs, "MemoryError");
    configureType(ejs, "OutOfBoundsError");
    configureType(ejs, "ReferenceError");
    configureType(ejs, "ResourceError");
    configureType(ejs, "SecurityError");
    configureType(ejs, "StateError");
    configureType(ejs, "SyntaxError");
    configureType(ejs, "TypeError");
    configureType(ejs, "URIError");

    ejsBindAccess(ejs, ejs->errorType, ES_Error_code, (EjsProc) getCode, (EjsProc) setCode);
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
