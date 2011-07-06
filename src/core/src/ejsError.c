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
static EjsAny *castError(Ejs *ejs, EjsError *error, EjsType *type)
{
    EjsString   *stack, *msg;
    EjsString   *us;
    char        *buf;

    switch (type->sid) {
    case S_Boolean:
        return ejsCreateBoolean(ejs, 1);

    case S_String:
        stack = (EjsString*) ejsRunFunctionBySlot(ejs, error, ES_Error_formatStack, 0, NULL);
        us = ejsIs(ejs, stack, String) ? stack : ESV(empty);
        msg = ejsGetProperty(ejs, error, ES_Error_message);
        if ((buf = mprAsprintf("%@ Exception: %@\nStack:\n%@\n", TYPE(error)->qname.name, msg, us)) == NULL) {
            ejsThrowMemoryError(ejs);
        }
        return ejsCreateStringFromAsc(ejs, buf);
        break;

    default:
        ejsThrowTypeError(ejs, "Unknown type");
    }
    return 0;
}


/*********************************** Methods **********************************/
/*
    Error Constructor and constructor for all the core error classes.

    public function Error(message: String = null)
 */
static EjsError *errorConstructor(Ejs *ejs, EjsError *error, int argc, EjsObj **argv)
{
    if (argc > 0) {
        ejsSetProperty(ejs, error, ES_Error_message, ejsToString(ejs, argv[0]));
    }
    if (ESV(Date)) {
        ejsSetProperty(ejs, error, ES_Error_timestamp, ejsCreateDate(ejs, mprGetTime()));
        ejsSetProperty(ejs, error, ES_Error_stack, ejsCaptureStack(ejs, 0));
    }
    return error;
}


/*
    static function capture(uplevels: Number): Array
 */
static EjsArray *error_capture(Ejs *ejs, EjsError *error, int argc,  EjsObj **argv)
{
    int     uplevels;
    
    uplevels = (argc > 0) ? ejsGetInt(ejs, argv[0]) : 0;
    return ejsCaptureStack(ejs, uplevels);
}

/************************************ Factory *********************************/

EjsError *ejsCreateError(Ejs *ejs, EjsType *type, EjsObj *msg) 
{
    EjsError    *error;

    error = ejsCreatePot(ejs, type, 0);
    if (error) {
        ejsSetProperty(ejs, error, ES_Error_message, msg);
        ejsSetProperty(ejs, error, ES_Error_timestamp, ejsCreateDate(ejs, mprGetTime()));
        ejsSetProperty(ejs, error, ES_Error_stack, ejsCaptureStack(ejs, 0));
    }
    return error;
}


static EjsType *defineType(Ejs *ejs, cchar *name, int id)
{
    EjsType     *type;

    type = ejsCreateCoreType(ejs, N("ejs", name), sizeof(EjsError), id, ES_Error_NUM_CLASS_PROP, ejsManagePot, 
        EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES | EJS_TYPE_HAS_INSTANCE_VARS | EJS_TYPE_MUTABLE_INSTANCES);
    type->constructor.block.nobind = 1;
    type->helpers.cast = (EjsCastHelper) castError;
    return type;
}


void ejsCreateErrorType(Ejs *ejs)
{
    defineType(ejs, "Error", S_Error);
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

    if ((type = ejsFinalizeCoreType(ejs, N("ejs", name))) == 0) {
        return;
    }
    ejsBindConstructor(ejs, type, errorConstructor);
}


void ejsConfigureErrorType(Ejs *ejs)
{
    //  OPT simplify
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

    ejsBindMethod(ejs, ESV(Error), ES_Error_capture, error_capture);
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
