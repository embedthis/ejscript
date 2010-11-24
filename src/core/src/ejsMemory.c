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
static EjsObj *getAllocatedMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return (EjsObj*) ejsCreateNumber(ejs, (int) mem->bytesAllocated);
}


#if UNUSED
/*
    native static function callback(fn: Function): Void
 */
static EjsObj *setRedlineCallback(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsFunction(ejs, argv[0]));

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
static EjsObj *getMaxMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return (EjsObj*) ejsCreateNumber(ejs, (int) mem->maxMemory);
}


/*
    native static function set maximum(limit: Number): Void
 */
static EjsObj *setMaxMemory(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     maxMemory;

    mprAssert(argc == 1 && ejsIsNumber(ejs, argv[0]));

    maxMemory = ejsGetInt(ejs, argv[0]);
    mprSetMemLimits(ejs, -1, maxMemory);
    return 0;
}


/*
    native static function get redline(): Number
 */
static EjsObj *getRedline(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return (EjsObj*) ejsCreateNumber(ejs, (int) mem->redLine);
}


/*
    native static function set redline(limit: Number): Void
 */
static EjsObj *setRedline(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     redline;

    mprAssert(argc == 1 && ejsIsNumber(ejs, argv[0]));

    redline = ejsGetInt(ejs, argv[0]);
    if (redline <= 0) {
        //  TODO - 64 bit
        redline = INT_MAX;
    }
    mprSetMemLimits(ejs, redline, -1);
    return 0;
}


/*
    native static function get resident(): Number
 */
static EjsObj *getResident(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return (EjsObj*) ejsCreateNumber(ejs, (int) mem->rss);
}


/*
    native static function get system(): Number
 */
static EjsObj *getSystemRam(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    MprMemStats    *mem;

    mem = mprGetMemStats(ejs);
    return (EjsObj*) ejsCreateNumber(ejs, (double) mem->ram);
}


/*
    native static function stats(): Void
 */
static EjsObj *printStats(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    //  MOB TODO - should go to log file and not to stdout
    mprPrintMemReport("Memory Report", 0);
    return 0;
}


/******************************** Initialization ******************************/

void ejsConfigureMemoryType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsGetTypeByName(ejs, N(EJS_EJS_NAMESPACE, "Memory"))) == 0) {
        mprError(ejs, "Can't find Memory type");
        return;
    }
    ejsBindMethod(ejs, type, ES_Memory_allocated, (EjsProc) getAllocatedMemory);
    ejsBindAccess(ejs, type, ES_Memory_maximum, (EjsProc) getMaxMemory, (EjsProc) setMaxMemory);
    ejsBindAccess(ejs, type, ES_Memory_redline, (EjsProc) getRedline, (EjsProc) setRedline);
    ejsBindMethod(ejs, type, ES_Memory_resident, (EjsProc) getResident);
    ejsBindMethod(ejs, type, ES_Memory_system, (EjsProc) getSystemRam);
    ejsBindMethod(ejs, type, ES_Memory_stats, (EjsProc) printStats);

#if UNUSED
    EjsPot      *prototype;
    prototype = type->prototype;
    ejsBindAccess(ejs, type, ES_Memory_callback, NULL, (EjsProc) setRedlineCallback);
#endif
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
