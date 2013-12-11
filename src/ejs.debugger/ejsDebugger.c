/*
    ejsDebugger.c -- 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/

/************************************ Code ************************************/
/*
    DB Constructor and also used for constructor for sub classes.

    function Debugger(connectionString: String)
 */
static EjsVar *debuggerConstructor(Ejs *ejs, EjsDebugger *db, int argc, EjsVar **argv)
{
    debugger3         *sdb;
    cchar           *path;

    path = ejsGetString(ejs, argv[0]);    
    db->ejs = ejs;
    
    /*
     *  Create a memory context for use by debugger. This is a virtual paged memory region.
     *  TODO OPT - Could do better for running applications.
     */
#if MAP_ALLOC
    db->arena = mprAllocArena(ejs, "debugger", EJS_MAX_SQLITE_MEM, !USE_TLS, 0);
    if (db->arena == 0) {
        return 0;
    }
    SET_CTX(db->arena);
#else
    db->arena = mprAllocHeap(ejs, "debugger", EJS_MAX_SQLITE_MEM, 1, 0);
    if (db->arena == 0) {
        return 0;
    }
    SET_CTX(db->arena);
#endif
    
#if UNUSED
    EjsDebugger       **dbp;
    /*
     *  Create a destructor object so we can cleanup and close the database. Must create after the arena so it will be
     *  invoked before the arena is freed. 
     */
    if ((dbp = mprAllocWithDestructor(ejs, sizeof(void*), (MprDestructor) sqldbDestructor)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    *dbp = db;
#endif

    sdb = 0;
    if (debugger3_open(path, &sdb) != SQLITE_OK) {
        ejsThrowIOError(ejs, "Cannot open database %s", path);
        return 0;
    }
    db->sdb = sdb;
    debugger3_busy_timeout(sdb, BIT_MAX_SQLITE_DURATION);
    debugger3_soft_heap_limit(BIT_MAX_SQLITE_MEM);
    return 0;
}

/*********************************** Factory *******************************/

static int configureDebuggerTypes(Ejs *ejs)
{
    EjsType         *type;
    
    type = (EjsType*) ejsConfigureNativeType(ejs, "ejs.db", "Debugger", sizeof(EjsDebugger));
    type->needFinalize = 1;

    type->helpers = ejsCloneObjectHelpers(ejs, "debugger-helpers");
    type->helpers->destroy = (EjsDestroyHelper) destroyDebuggerDb;

    ejsBindMethod(ejs, type, ES_ejs_db_Debugger_Debugger, debuggerConstructor);
    ejsBindMethod(ejs, type, ES_ejs_db_Debugger_close, debuggerClose);
    ejsBindMethod(ejs, type, ES_ejs_db_Debugger_sql, debuggerSql);
    return 0;
}


int ejs_db_debugger_Init(MprCtx ctx)
{
    return ejsAddNativeModule(ctx, "ejs.db.debugger", configureDebuggerTypes, _ES_CHECKSUM_ejs_db_debugger, 
        EJS_LOADER_ETERNAL);
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
