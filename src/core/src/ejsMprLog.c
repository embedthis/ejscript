/*
    ejsMprLog.c -- MprLog class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*
    function emit(level: Number, ...data): Number
 */
static EjsNumber *lf_emit(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *ap;
    EjsObj          *vp;
    EjsString       *str;
    char            *msg, *arg;
    ssize           len, written;
    int             i, level, paused;

    mprAssert(argc >= 2 && ejsIs(ejs, argv[1], Array));

    level = ejsGetInt(ejs, argv[0]);
    args = (EjsArray*) argv[1];
    written = 0;
    msg = 0;
    paused = ejsPauseGC(ejs);

    for (i = 0; i < args->length; i++) {
        vp = ejsGetProperty(ejs, args, i);
        mprAssert(vp);
        switch (TYPE(vp)->sid) {
        case S_ByteArray:
            ap = (EjsByteArray*) vp;
            //  TODO ENCODING
            arg = (char*) &ap->value[ap->readPosition];
            len = ap->writePosition - ap->readPosition;
            break;

        case S_String:
            arg = awtom(((EjsString*) vp)->value, &len);
            break;

        default:
            str = ejsToString(ejs, vp);
            arg = awtom(((EjsString*) str)->value, &len);
            break;
        }
        msg = srejoin(msg, arg, NULL);
    }
    if (msg) {
        mprRawLog(level, "%s", msg);
        written += slen(msg);
    }
    ejsResumeGC(ejs, paused);
    return ejsCreateNumber(ejs, (MprNumber) slen(msg));
}


/*  
    function get cmdline(): Boolean
 */
static EjsBoolean *lf_cmdline(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, mprGetCmdlineLogging());
}


/*  
    function set cmdline(yes: Boolean)
 */
static EjsVoid *lf_set_cmdline(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    int     yes;

    yes = (argc >= 1 && argv[0] == ESV(true));
    mprSetCmdlineLogging(yes);
    return 0;
}


/*  
    function get level(): Number
 */
static EjsNumber *lf_level(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, mprGetLogLevel(ejs));
}


/*  
    function set level(value: Number): Void
 */
static EjsObj *lf_set_level(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprSetLogLevel(ejsGetInt(ejs, argv[0]));
    return 0;
}


/*  
    function redirect(location: String, level: Number = null): Void
 */
static EjsFile *lf_redirect(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *logSpec;
    int     level;

    logSpec = ejsToMulti(ejs, argv[0]);
    level = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : -1;
    ejsRedirectLogging(logSpec);
    if (level >= 0) {
        mprSetLogLevel(level);
    }
    return 0;
}


/*********************************** Factory **********************************/

void ejsConfigureMprLogType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "MprLog"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindAccess(ejs, prototype, ES_MprLog_cmdline, lf_cmdline, lf_set_cmdline);
    ejsBindMethod(ejs, prototype, ES_MprLog_emit, lf_emit);
    ejsBindAccess(ejs, prototype, ES_MprLog_level, lf_level, lf_set_level);
    ejsBindMethod(ejs, prototype, ES_MprLog_redirect, lf_redirect);
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
