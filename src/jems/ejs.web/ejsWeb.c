/*
    ejsWeb.c -- Ejscript web framework.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsCompiler.h"
#include    "ejsWeb.h"
#include    "ejs.web.slots.h"

/***************************** Forward Declarations ***************************/

static int configureWebTypes(Ejs *ejs);

/************************************ Code ************************************/
/*  
    HTML escape a string
    function escapeHtml(str: String): String
 */
static EjsObj *web_escapeHtml(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *str;

    str = (EjsString*) argv[0];
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprEscapeHtml(str->value));
}


static int configureWebTypes(Ejs *ejs)
{
    int         slotNum;

    if ((slotNum = ejsLookupProperty(ejs, ejs->global, N("ejs.web", "escapeHtml"))) != 0) {
        ejsBindFunction(ejs, ejs->global, slotNum, web_escapeHtml);
    }
    ejsConfigureHttpServerType(ejs);
    ejsConfigureRequestType(ejs);
    ejsConfigureSessionType(ejs);
    return 0;
}


/*  
    Module load entry point. This must be idempotent as it will be called for each new interpreter created.
 */
int ejs_web_Init(Ejs *ejs, MprModule *mp)
{
    return ejsAddNativeModule(ejs, "ejs.web", configureWebTypes, _ES_CHECKSUM_ejs_web, EJS_LOADER_ETERNAL);
}


/*
    @copy   default
 
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
 
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.
 
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html
 
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
 
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
