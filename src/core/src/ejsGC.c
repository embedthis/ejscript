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
static EjsBoolean *gc_enabled(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    return ((mprGetMpr()->heap->gcEnabled) ? ESV(true): ESV(false));
}


/*
    native static function set enabled(on: Boolean): Void
 */
static EjsObj *gc_set_enabled(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    assert(argc == 1 && ejsIs(ejs, argv[0], Boolean));
    mprGetMpr()->heap->gcEnabled = ejsGetBoolean(ejs, argv[0]);
    return 0;
}


/*
    run()
 */
static EjsObj *gc_run(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    assert(!ejs->state->paused);
    
    if (!ejs->state->paused) {
        mprGC(MPR_GC_FORCE);
    }
    return 0;
}


/*
    native static function get newQuota(): Number
 */
static EjsNumber *gc_newQuota(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) mprGetMpr()->heap->workQuota);
}


/*
    native static function set newQuota(quota: Number): Void
 */
static EjsObj *gc_set_newQuota(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    int     quota;

    assert(argc == 1 && ejsIs(ejs, argv[0], Number));
    quota = ejsGetInt(ejs, argv[0]);

    if (quota < 1024 && quota != 0) {
        ejsThrowArgError(ejs, "Bad work quota. Must be > 1024");
        return 0;
    }
    mprGetMpr()->heap->workQuota = quota;
    return 0;
}


/*
    verify(): Void
 */
static EjsObj *gc_verify(Ejs *ejs, EjsObj *thisObj, int argc, EjsObj **argv)
{
    //  UNUSED - not supported
    return 0;
}


PUBLIC void ejsConfigureGCType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "GC"), sizeof(EjsPot), ejsManagePot, EJS_TYPE_POT)) == 0) {
        return;
    }
    ejsBindAccess(ejs, type, ES_GC_enabled, gc_enabled, gc_set_enabled);
    ejsBindAccess(ejs, type, ES_GC_newQuota, gc_newQuota, gc_set_newQuota);
    ejsBindMethod(ejs, type, ES_GC_run, gc_run);
    ejsBindMethod(ejs, type, ES_GC_verify, gc_verify);
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
