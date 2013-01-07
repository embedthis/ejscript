/*
    ejsMemory.c - Memory class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods *********************************/
/*
    native static function get allocated(): Number
 */
static EjsNumber *getAllocatedMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return ejsCreateNumber(ejs, (MprNumber) mem->bytesAllocated);
}


#if FUTURE
/*
    native static function callback(fn: Function): Void
 */
static EjsNumber *setRedlineCallback(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    assert(argc == 1 && ejsIsFunction(ejs, argv[0]));

    if (!ejsIsFunction(ejs, argv[0])) {
        ejsThrowArgError(ejs, "Callaback is not a function");
        return 0;
    }
    ejs->memoryCallback = (EjsFunction*) argv[0];
    return 0;
}
#endif


/*
    native static function get maximum(): Number
 */
static EjsNumber *getMaxMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return ejsCreateNumber(ejs, (MprNumber) mem->maxMemory);
}


/*
    native static function set maximum(limit: Number): Void
 */
static EjsObj *setMaxMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     maxMemory;

    assert(argc == 1 && ejsIs(ejs, argv[0], Number));

    maxMemory = ejsGetInt(ejs, argv[0]);
    mprSetMemLimits(-1, maxMemory);
    return 0;
}


/*
    native static function get redline(): Number
 */
static EjsNumber *getRedline(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return ejsCreateNumber(ejs, (MprNumber) mem->redLine);
}


/*
    native static function set redline(limit: Number): Void
 */
static EjsObj *setRedline(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     redline;

    assert(argc == 1 && ejsIs(ejs, argv[0], Number));

    redline = ejsGetInt(ejs, argv[0]);
    if (redline <= 0) {
        //  TODO - 64 bit
        redline = MAXINT;
    }
    mprSetMemLimits(redline, -1);
    return 0;
}


/*
    native static function get resident(): Number
 */
static EjsNumber *getResident(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return ejsCreateNumber(ejs, (MprNumber) mem->rss);
}


/*
    native static function get system(): Number
 */
static EjsNumber *getSystemRam(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return ejsCreateNumber(ejs, (double) mem->ram);
}


/*
    native static function stats(): Void
 */
static EjsObj *printStats(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    //  TODO - should go to log file and not to stdout
    mprPrintMem("Memory Report", 1);
    return 0;
}


/******************************** Initialization ******************************/

PUBLIC void ejsConfigureMemoryType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Memory"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT)) == 0) {
        return;
    }
    ejsBindMethod(ejs, type, ES_Memory_allocated, getAllocatedMemory);
    ejsBindAccess(ejs, type, ES_Memory_maximum, getMaxMemory, setMaxMemory);
    ejsBindAccess(ejs, type, ES_Memory_redline, getRedline, setRedline);
    ejsBindMethod(ejs, type, ES_Memory_resident, getResident);
    ejsBindMethod(ejs, type, ES_Memory_system, getSystemRam);
    ejsBindMethod(ejs, type, ES_Memory_stats, printStats);

#if FUTURE
    EjsPot      *prototype;
    prototype = type->prototype;
    ejsBindAccess(ejs, type, ES_Memory_callback, NULL, setRedlineCallback);
#endif
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
