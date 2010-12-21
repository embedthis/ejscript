/*
    ejsLogger.c -- Logger class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*  
    function get nativeLevel(): Number
 */
static EjsObj *logger_nativeLevel(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, mprGetLogLevel(ejs));
}


/*  
    function set nativeLevel(value: Number): Void
 */
static EjsObj *logger_set_nativeLevel(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprSetLogLevel(ejsGetInt(ejs, argv[0]));
    return 0;
}


/*  
    function get nativeStream(): Stream
 */
static EjsObj *logger_nativeStream(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    int     fd;

    if ((fd = mprGetLogFd(ejs)) >= 0) {
        return (EjsObj*) ejsCreateFileFromFd(ejs, fd, "mpr-logger", O_WRONLY);
    }
    return (EjsObj*) ejs->nullValue;
}


/*********************************** Factory **********************************/

void ejsConfigureLoggerType(Ejs *ejs)
{
    EjsType         *type;

    type = ejsGetTypeByName(ejs, N("ejs", "Logger"));
    mprAssert(type);

    ejsBindAccess(ejs, type, ES_Logger_nativeLevel, (EjsProc) logger_nativeLevel, (EjsProc) logger_set_nativeLevel);
    ejsBindMethod(ejs, type, ES_Logger_nativeStream, (EjsProc) logger_nativeStream);
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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
