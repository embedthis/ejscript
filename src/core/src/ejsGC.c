/**
    ejsGC.c - Garbage collector class for the EJS Object Model

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    native static function get enabled(): Boolean
 */
static EjsObj *getEnable(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    return (EjsObj*) ((ejs->heap->enabled) ? ejs->trueValue: ejs->falseValue);
}


/*
    native static function set enabled(on: Boolean): Void
 */
static EjsObj *setEnable(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    mprAssert(argc == 1 && ejsIsBoolean(ejs, argv[0]));
    ejs->heap->enabled = ejsGetBoolean(ejs, argv[0]);
    return 0;
}


/*
    run(deep: Boolean = false)
    Note: deep currently is not implemented
 */
static EjsObj *runGC(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     deep;

    deep = ((argc == 1) && ejsIsBoolean(ejs, argv[1]));
    ejsCollectGarbage(ejs, EJS_GEN_NEW);
    return 0;
}


/*
    native static function get workQuota(): Number
 */
static EjsObj *getWorkQuota(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, ejs->heap->workQuota);
}


/*
    native static function set workQuota(quota: Number): Void
 */
static EjsObj *setWorkQuota(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     quota;

    mprAssert(argc == 1 && ejsIsNumber(ejs, argv[0]));
    quota = ejsGetInt(ejs, argv[0]);

    if (quota < EJS_SHORT_WORK_QUOTA && quota != 0) {
        ejsThrowArgError(ejs, "Bad work quota");
        return 0;
    }
    ejs->heap->workQuota = quota;
    return 0;
}


void ejsConfigureGCType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsGetTypeByName(ejs, EJS_EJS_NAMESPACE, "GC")) == 0) {
        mprError(ejs, "Can't find GC type");
        return;
    }
    ejsBindAccess(ejs, type, ES_GC_enabled, (EjsProc) getEnable, (EjsProc) setEnable);
    ejsBindAccess(ejs, type, ES_GC_workQuota, (EjsProc) getWorkQuota, (EjsProc) setWorkQuota);
    ejsBindMethod(ejs, type, ES_GC_run, (EjsProc) runGC);
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
