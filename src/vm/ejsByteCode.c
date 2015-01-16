/**
    ejsByteCode.c - Definition of the byte code table.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/
/*
    This will define an instance of the EjsOptable which is defined in ejsByteCodeTable.h
 */
#define EJS_DEFINE_OPTABLE 1

#include    "ejs.h"

PUBLIC EjsOptable *ejsGetOptable()
{
    return ejsOptable;
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
