/*
    ejsDebug.c - Debug.Debug class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    Trap to the debugger

    static function breakpoint(): Void
 */
static EjsObj *debug_breakpoint(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if BLD_DEBUG && DEBUG_IDE && 0
    #if BLD_WIN_LIKE && !MPR_64_BIT
        __asm { int 3 };
    #elif (MACOSX || LINUX) && (BLD_HOST_CPU_ARCH == MPR_CPU_IX86 || BLD_HOST_CPU_ARCH == MPR_CPU_IX64)
        asm("int $03");
        /*  __asm__ __volatile__ ("int $03"); */
    #endif
#else
    mprBreakpoint();
#endif
    return 0;
}


/*
    function get mode(): Boolean
 */
static EjsObj *debug_mode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return mprGetDebugMode() ? S(true) : S(false);
}


/*
    function set mode(on: Boolean): Void
 */
static EjsObj *debug_set_mode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprSetDebugMode(argv[0] == S(true));
    return 0;
}

/************************************ Factory *********************************/

void ejsConfigureDebugType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsGetTypeByName(ejs, N("ejs", "Debug"))) == 0) {
        mprError("Can't find Debug type");
        return;
    }
    ejsBindMethod(ejs, type, ES_Debug_breakpoint, debug_breakpoint);
    ejsBindAccess(ejs, type, ES_Debug_mode, debug_mode, debug_set_mode);

    ejsBindFunction(ejs, ejs->global, ES_breakpoint, debug_breakpoint);
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
