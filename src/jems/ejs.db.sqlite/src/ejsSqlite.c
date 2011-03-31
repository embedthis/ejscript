/*
    ejsSqlite.c -- SQLite Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.

    Todo:
        - should handle SQLITE_BUSY for multiuser access. Need to set the default timeout
        Useful: SQLITE_API sqlite3_int64 sqlite3_last_insert_rowid(sqlite3*);
 */
/********************************** Includes **********************************/

#include    "ejs.h"

#if BLD_FEATURE_SQLITE

#include    "sqlite3.h"
#include    "ejs.db.sqlite.slots.h"

/*********************************** Locals ***********************************/

#define THREAD_STYLE SQLITE_CONFIG_MULTITHREAD
//#define THREAD_STYLE SQLITE_CONFIG_SERIALIZED

/*
    Map allocation and mutex routines. Can't use this (yet) as allocated memory must be marked
 */
#define MAP_ALLOC   1
#define MAP_MUTEXES 1

/*
    Ejscript Sqlite class object
 */
typedef struct EjsSqlite {
    EjsPot          pot;            /* Extends Object */
    sqlite3         *sdb;           /* Sqlite handle */
    Ejs             *ejs;           /* Interp reference */
    int             memory;         /* In-memory database */
} EjsSqlite;

/************************************ Code ************************************/
/*
    DB Constructor and also used for constructor for sub classes.

    function Sqlite(options: Object)
 */
static EjsObj *sqliteConstructor(Ejs *ejs, EjsSqlite *db, int argc, EjsObj **argv)
{
    sqlite3         *sdb;
    EjsObj          *options;
    cchar           *path;

    sdb = 0;
    db->ejs = ejs;
    options = argv[0];
    
    /*
        MOB - this will create a database if it doesn't exist. Should have more control over creating databases.
     */
    if (ejsIs(ejs, options, Path) || ejsIs(ejs, options, String)) {
        path = ejsToMulti(ejs, ejsToString(ejs, options));
    } else {
        path = ejsToMulti(ejs, ejsToString(ejs, ejsGetPropertyByName(ejs, options, EN("name"))));
    }
    if (strncmp(path, "memory://", 9) == 0) {
        sdb = (sqlite3*) (size_t) stoi(&path[9], 10, NULL);

    } else {
        db->memory = 0;
        if (strncmp(path, "file://", 7) == 0) {
            path += 7;
        }
        if (strstr(path, "://") == NULL) {
            if (sqlite3_open(path, &sdb) != SQLITE_OK) {
                ejsThrowIOError(ejs, "Can't open database %s", path);
                return 0;
            }
            //  MOB TODO - should be configurable somewhere
            sqlite3_soft_heap_limit(20 * 1024 * 1024);
            sqlite3_busy_timeout(sdb, EJS_SQLITE_TIMEOUT);

        } else {
            ejsThrowArgError(ejs, "Unknown SQLite database URI %s", path);
            return 0;
        }
    }
    db->sdb = sdb;
    return (EjsObj*) db;
}


/*
    function close(): Void
 */
static int sqliteClose(Ejs *ejs, EjsSqlite *db, int argc, EjsObj **argv)
{
    mprAssert(ejs);
    mprAssert(db);

    if (db->sdb && !db->memory) {
        sqlite3_close(db->sdb);
        db->sdb = 0;
    }
    return 0;
}


/*
    function sql(cmd: String): Array

    Will support multiple sql cmds but will only return one result table.
 */
static EjsObj *sqliteSql(Ejs *ejs, EjsSqlite *db, int argc, EjsObj **argv)
{
    sqlite3         *sdb;
    sqlite3_stmt    *stmt;
    EjsArray        *result;
    EjsObj          *row;
    EjsObj          *svalue;
    EjsName         qname;
    char            *tableName;
    cchar           *tail, *colName, *cmd, *value, *defaultTableName;
    int             i, ncol, rc, retries, rowNum, len;

    mprAssert(ejs);
    mprAssert(db);

    cmd = ejsToMulti(ejs, argv[0]);
    retries = 0;
    sdb = db->sdb;
    if (sdb == 0) {
        ejsThrowIOError(ejs, "Database is closed");
        return 0;
    }
    mprAssert(sdb);

    result = ejsCreateArray(ejs, 0);
    if (result == 0) {
        return 0;
    }

    rc = SQLITE_OK;
    while (cmd && *cmd && (rc == SQLITE_OK || (rc == SQLITE_SCHEMA && ++retries < 2))) {
        stmt = 0;
        rc = sqlite3_prepare_v2(sdb, cmd, -1, &stmt, &tail);
        if (rc != SQLITE_OK) {
            continue;
        }
        if (stmt == 0) {
            /* Comment or white space */
            cmd = tail;
            continue;
        }
        defaultTableName = 0;
        ncol = sqlite3_column_count(stmt);
        for (rowNum = 0; ; rowNum++) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                row = ejsCreateEmptyPot(ejs);
                if (row == 0) {
                    sqlite3_finalize(stmt);
                    return 0;
                }
                if (ejsSetProperty(ejs, (EjsObj*) result, rowNum, (EjsObj*) row) < 0) {
                    ejsThrowIOError(ejs, "Can't update query result set");
                    return 0;
                }
#if MOB
                /* This will ensure we have a row->names memory context. This gets freed if the object is pooled. */
                ejsGrowObjectNames(row, rowNum + 1);
#endif
                for (i = 0; i < ncol; i++) {
                    tableName = (char*) sqlite3_column_table_name(stmt, i);
                    if (defaultTableName == 0) {
                        defaultTableName = tableName;
                    }
                    colName = sqlite3_column_name(stmt, i);
                    value = (cchar*) sqlite3_column_text(stmt, i);

                    if (tableName == 0 || strcmp(tableName, defaultTableName) == 0) {
                        qname = EN(colName);
                    } else {
                        /*
                            Append the table name for columns from foreign tables. Convert to camel case (tableColumn)
                            MOB - refactor crude singularization.
                         */
                        len = strlen(tableName) + 1;
                        tableName = sjoin("_", tableName, colName, NULL);
                        if (len > 3 && tableName[len - 1] == 's' && tableName[len - 2] == 'e' && tableName[len - 3] == 'i') {
                            tableName[len - 3] = 'y';
                            strcpy(&tableName[len - 2], colName);
                            len -= 2;
                        } else if (len > 2 && tableName[len - 1] == 's' && tableName[len - 2] == 'e') {
                            strcpy(&tableName[len - 2], colName);
                            len -= 2;
                        } else if (tableName[len - 1] == 's') {
                            strcpy(&tableName[len - 1], colName);
                            len--;
                        }
                        // tableName[0] = tolower((int) tableName[0]);
                        tableName[len] = toupper((int) tableName[len]);
                        qname = EN(tableName);
                    }
                    if (ejsLookupProperty(ejs, (EjsObj*) row, qname) < 0) {
                        svalue = (EjsObj*) ejsCreateStringFromMulti(ejs, value, slen(value));
                        if (ejsSetPropertyByName(ejs, (EjsObj*) row, qname, svalue) < 0) {
                            ejsThrowIOError(ejs, "Can't update query result set name");
                            return 0;
                        }
                    }
                }
            } else {
                rc = sqlite3_finalize(stmt);
                stmt = 0;
                if (rc != SQLITE_SCHEMA) {
                    retries = 0;
                    for (cmd = tail; isspace((int) *cmd); cmd++) {
                        ;
                    }
                }
                break;
            }
        }
    }
    if (stmt) {
        rc = sqlite3_finalize(stmt);
    }
    if (rc != SQLITE_OK) {
        if (rc == sqlite3_errcode(sdb)) {
            ejsThrowIOError(ejs, "SQL error: %s", sqlite3_errmsg(sdb));
        } else {
            ejsThrowIOError(ejs, "Unspecified SQL error");
        }
        return 0;
    }
    return (EjsObj*) result;
}


/*********************************** Alloc ********************************/
#if MAP_ALLOC

static void *allocBlock(int size)
{
    void    *ptr;

    if ((ptr = mprAlloc(size)) != 0) {
        mprHold(ptr);
    }
    return ptr;
}


static void freeBlock(void *ptr)
{
    mprRelease(ptr);
}


static void *reallocBlock(void *ptr, int size)
{
    mprRelease(ptr);
    if ((ptr =  mprRealloc(ptr, size)) != 0) {
        mprHold(ptr);
    }
    return ptr;
}


static int blockSize(void *ptr)
{
    return mprGetBlockSize(ptr);
}


static int roundBlockSize(int size)
{
    return MPR_ALLOC_ALIGN(size);
}


static int initAllocator(void *data)
{
    return 0;
}


static void termAllocator(void *data)
{
}


struct sqlite3_mem_methods mem = {
    allocBlock, freeBlock, reallocBlock, blockSize, roundBlockSize, initAllocator, termAllocator, NULL 
};

#endif /* MAP_ALLOC */

/*********************************** Mutex ********************************/
#if MAP_MUTEXES

static int initMutex(void) { 
    return 0; 
}


static int termMutex(void) { 
    return 0; 
}


static sqlite3_mutex *allocMutex(int kind)
{
    MprMutex    *lock;

    if ((lock = mprCreateLock()) != 0) {
        mprHold(lock);
    }
    return (sqlite3_mutex*) lock;
}


static void freeMutex(sqlite3_mutex *mutex)
{
    mprRelease((MprMutex*) mutex);
}


static void enterMutex(sqlite3_mutex *mutex)
{
    mprLock((MprMutex*) mutex);
}


static int tryMutex(sqlite3_mutex *mutex)
{
    return mprTryLock((MprMutex*) mutex);
}


static void leaveMutex(sqlite3_mutex *mutex)
{
    mprUnlock((MprMutex*) mutex);
}


static int mutexIsHeld(sqlite3_mutex *mutex) { 
    mprAssert(0); 
    return 0; 
}


static int mutexIsNotHeld(sqlite3_mutex *mutex) { 
    mprAssert(0); 
    return 0; 
}


struct sqlite3_mutex_methods mut = {
    initMutex, termMutex, allocMutex, freeMutex, enterMutex, tryMutex, leaveMutex, mutexIsHeld, mutexIsNotHeld,
};

#endif /* MAP_MUTEXES */

/*********************************** Factory *******************************/

static int manageSqlite(EjsSqlite *db, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(db, flags);

    } else if (flags & MPR_MANAGE_FREE) {
        if (db->sdb) {
            sqliteClose(db->ejs, db, 0, 0);
        }
    }
    return 0;
}


static int configureSqliteTypes(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;
    static int  initialized = 0;
    
    type = (EjsType*) ejsConfigureNativeType(ejs, N("ejs.db", "Sqlite"), sizeof(EjsSqlite), manageSqlite, EJS_POT_HELPERS);
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, (EjsProc) sqliteConstructor);
    ejsBindMethod(ejs, prototype, ES_ejs_db_Sqlite_close, (EjsProc) sqliteClose);
    ejsBindMethod(ejs, prototype, ES_ejs_db_Sqlite_sql, (EjsProc) sqliteSql);

    if (!initialized) {
        initialized++;
#if MAP_ALLOC
        sqlite3_config(SQLITE_CONFIG_MALLOC, &mem);
#endif
#if MAP_MUTEXES
        sqlite3_config(SQLITE_CONFIG_MUTEX, &mut);
#endif
        sqlite3_config(THREAD_STYLE);
        if (sqlite3_initialize() != SQLITE_OK) {
            mprError("Can't initialize SQLite");
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    return 0;
}


/*
    Module load entry point. This must be idempotent as it will be called for each new interpreter created.
 */
int ejs_db_sqlite_Init(Ejs *ejs, MprModule *mp)
{
    return ejsAddNativeModule(ejs, "ejs.db.sqlite", configureSqliteTypes, _ES_CHECKSUM_ejs_db_sqlite, EJS_LOADER_ETERNAL);
}

#endif /* BLD_FEATURE_SQLITE */
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
