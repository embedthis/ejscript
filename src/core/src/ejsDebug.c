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
#if BIT_DEBUG && DEBUG_IDE
    #if BIT_WIN_LIKE && !BIT_64
        __asm { int 3 };
    #elif (MACOSX || LINUX) && (BIT_CPU_ARCH == MPR_CPU_IX86 || BIT_CPU_ARCH == MPR_CPU_IX64)
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
    return mprGetDebugMode() ? ESV(true) : ESV(false);
}


/*
    function set mode(on: Boolean): Void
 */
static EjsObj *debug_set_mode(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    mprSetDebugMode(argv[0] == ESV(true));
    return 0;
}

/************************************ Factory *********************************/

PUBLIC void ejsConfigureDebugType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Debug"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT)) != 0) {
        ejsBindMethod(ejs, type, ES_Debug_breakpoint, debug_breakpoint);
        ejsBindAccess(ejs, type, ES_Debug_mode, debug_mode, debug_set_mode);
    }
    ejsBindFunction(ejs, ejs->global, ES_breakpoint, debug_breakpoint);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
