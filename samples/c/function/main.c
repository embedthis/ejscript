/*
    main.c - Simple main program to load and run a module
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Code ************************************/

static EjsObj *hello(Ejs *ejs, EjsAny *thisObj, int argc, EjsObj **argv)
{
    printf("Hello World\n");
    return 0;
}


static EjsObj *message(Ejs *ejs, EjsAny *thisObj, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, "Hello Cruel World", -1);
}


int main(int argc, char **argv)
{
    Ejs     *ejs;

    mprCreate(0, 0, 0);

    if (argc != 1) {
        mprError("usage: main test.es");
        return 255;
    }
    if ((ejs = ejsCreateVM(0, 0, 0)) == 0) {
        mprDestroy(0);
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ejs);
    if (ejsLoadModules(ejs, 0, 0) < 0) {
        mprDestroy(0);
        return MPR_ERR_CANT_READ;
    }
    ejsDefineGlobalFunction(ejs, ejsCreateString(ejs, "hello", -1), hello);
    ejsDefineGlobalFunction(ejs, ejsCreateString(ejs, "message", -1), message);

    if (ejsLoadScriptFile(ejs, argv[1], NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG) < 0) {
        ejsReportError(ejs, "Error in program");
        mprDestroy(0);
        return MPR_ERR;
    }
    mprDestroy(MPR_EXIT_DEFAULT);
    return 0;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
