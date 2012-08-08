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
        ejsThrowIOError(ejs, "Can't open database %s", path);
        return 0;
    }
    db->sdb = sdb;
    debugger3_busy_timeout(sdb, EJS_SQLITE_TIMEOUT);

    //  TODO - should be configurable somewhere
    debugger3_soft_heap_limit(2 * 1024 * 1024);
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

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
