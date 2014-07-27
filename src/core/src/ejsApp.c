/*
    ejsApp.c -- App class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*  
    Get the application command line arguments
    static function get args(): Array
 */
static EjsArray *app_args(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsArray    *args;
    int         i;

    args = ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, ejsCreateStringFromAsc(ejs, ejs->argv[i]));
    }
    return args;
}


/*  
    Get the current working directory
    function get dir(): Path
 */
static EjsPath *app_dir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetCurrentPath(ejs));
}


/*  
    Set the current working directory
    function chdir(value: String|Path): void
 */
static EjsObj *app_chdir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *path;

    assert(argc == 1);

    if (ejsIs(ejs, argv[0], Path)) {
        path = ((EjsPath*) argv[0])->value;

    } else if (ejsIs(ejs, argv[0], String)) {
        path = ejsToMulti(ejs, argv[0]);

    } else {
        ejsThrowIOError(ejs, "Bad path");
        return NULL;
    }
#if WINDOWS
{
    MprFileSystem   *fs;
    fs = mprLookupFileSystem(path);
    if (!mprPathExists(path, X_OK) && *path == '/') {
        path = sjoin(fs->cygwin, path, NULL);
    }
}
#endif
    if (chdir((char*) path) < 0) {
        ejsThrowIOError(ejs, "Cannot change the current directory");
    }
    return 0;
}

/*  
    Get the directory containing the application's executable file.
    static function get exeDir(): Path
 */
static EjsPath *app_exeDir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetAppDir());
}


/*  
    Get the application's executable filename.
    static function get exePath(): Path
 */
static EjsPath *app_exePath(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, mprGetAppPath());
}


/*  
    Exit the application
    static function exit(status: Number, how: String = "immediate"): void
    TODO - status is not implemented
 */
static EjsObj *app_exit(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprTicks    timeout;
    cchar       *how;
    int         status, mode;

    if (ejs->dontExit) {
        ejsThrowStateError(ejs, "App.exit has been disabled");
        return 0;
    }
    status = argc >= 1 ? ejsGetInt(ejs, argv[0]) : 0;
    timeout = argc >= 2 ? ejsGetInt(ejs, argv[1]) : 0;
    how = ejsToMulti(ejs, argc >= 2 ? ejsToString(ejs, argv[1]): ESV(empty));

    if (scmp(how, "normal") == 0) {
        mode = 0;
    } else if (scmp(how, "abort") == 0) {
        mode = MPR_EXIT_ABORT;
    } else if (scmp(how, "safe") == 0) {
        mode = MPR_EXIT_SAFE;
    } else if (scmp(how, "restart") == 0) {
        mode = MPR_EXIT_RESTART;
#if DEPRECATED || 1
    } else if (scmp(how, "immediate") == 0) {
        mode = 0;
    } else if (scmp(how, "graceful") == 0) {
        mode = 0;
        if (argc <= 2) {
            timeout = 30 * 3000;
        }
    } else if (scmp(how, "default") == 0) {
        mode = 0;
#endif
    } else {
        mode = 0;
    }
    mprShutdown(mode, status, timeout);
    ejsAttention(ejs);
    return 0;
}


#if ES_App_env
/*  
    Get all environment vars
    function get env(): Object
 */
static EjsAny *app_env(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
#if VXWORKS
    return ESV(null);
#else
    EjsPot  *result;
    char        **ep, *pair, *key, *value;

    result = ejsCreatePot(ejs, ESV(Object), 0);
    for (ep = environ; ep && *ep; ep++) {
        pair = sclone(*ep);
        key = stok(pair, "=", &value);
        ejsSetPropertyByName(ejs, result, EN(key), ejsCreateStringFromAsc(ejs, value));
    }
    return result;
#endif
}
#endif


/*  
    Get an environment var
    function getenv(key: String): String
 */
static EjsAny *app_getenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *value;

    value = getenv(ejsToMulti(ejs, argv[0]));
    if (value == 0) {
        return ESV(null);
    }
    return ejsCreateStringFromAsc(ejs, value);
}


/*
    static function get gid(): Number
 */
static EjsNumber *app_gid(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if ME_UNIX_LIKE
    return ejsCreateNumber(ejs, getgid());
#else
    return ESV(null);
#endif
}


/*  
    Put an environment var
    function putenv(key: String, value: String): void
 */
static EjsObj *app_putenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    char    *key, *value;

    key = sclone(ejsToMulti(ejs, argv[0]));
    value = sclone(ejsToMulti(ejs, argv[1]));
    mprSetEnv(key, value);
    return 0;
}


/*  
    Get the ejs module search path. Does not actually read the environment.
    function get search(): Array
 */
static EjsArray *app_search(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    return ejs->search;
}


/*  
    Set the ejs module search path. Does not actually update the environment.
    function set search(path: Array): Void
 */
static EjsObj *app_set_search(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    ejsSetSearchPath(ejs, (EjsArray*) argv[0]);
    return 0;
}


/*  
    Get a default search path. NOTE: this does not modify ejs->search.
    function get createSearch(searchPaths: String): Array
 */
static EjsArray *app_createSearch(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *searchPath;

    searchPath = (argc == 0) ? NULL : ejsToMulti(ejs, argv[0]);
    return ejsCreateSearchPath(ejs, searchPath);
}


/*
    static function get pid (): Number
 */
static EjsNumber *app_pid(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, getpid());
}


/*  
    static function run(timeout: Number = -1, oneEvent: Boolean = false): Boolean
 */
static EjsObj *app_run(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprTicks    mark, remaining;
    int64       dispatcherMark;
    int         rc, oneEvent, timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]) : MAXINT;
    oneEvent = (argc > 1) ? ejsGetInt(ejs, argv[1]) : 0;

    if (ejs->hosted) {
        return ESV(true);
    }
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTicks();
    remaining = timeout;
    dispatcherMark = mprGetEventMark(ejs->dispatcher);
    do {
        rc = mprWaitForEvent(ejs->dispatcher, remaining, dispatcherMark); 
        remaining = mprGetRemainingTicks(mark, timeout);
        dispatcherMark = mprGetEventMark(ejs->dispatcher);
    } while (!ejs->exception && !oneEvent && !ejs->exiting && remaining > 0 && !mprIsStopping());
    return (rc == 0) ? ESV(true) : ESV(false);
}


/*  
    Pause the application. This services events while asleep.
    static function sleep(delay: Number = -1): void
    TODO - sleep currently throws if an exception is generated in an event callback (worker).
    It should not.
 */
static EjsObj *app_sleep(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprTicks    mark, remaining;
    int64       dispatcherMark;
    int         timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]) : MAXINT;
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTicks();
    remaining = timeout;
    dispatcherMark = mprGetEventMark(ejs->dispatcher);
    do {
        mprWaitForEvent(ejs->dispatcher, remaining, dispatcherMark); 
        remaining = mprGetRemainingTicks(mark, timeout);
        dispatcherMark = mprGetEventMark(ejs->dispatcher);
    } while (!ejs->exiting && remaining > 0 && !mprIsStopping());
    return 0;
}


/*  
    static function get uid(): Number
 */
static EjsNumber *app_uid(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if ME_UNIX_LIKE
    return ejsCreateNumber(ejs, getuid());
#else
    return ESV(null);
#endif
}


/*  
    static function getpass(prompt: String): String
 */
static EjsString *app_getpass(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprGetPassword((argc > 0) ? ejsToMulti(ejs, argv[0]) : "Password: "));
}


/*********************************** Factory **********************************/

PUBLIC void ejsConfigureAppType(Ejs *ejs)
{
    EjsType     *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "App"), 0, 0, 0)) == 0) {
        return;
    }
    ejsSetProperty(ejs, type, ES_App__inputStream, ejsCreateFileFromFd(ejs, 0, "stdin", O_RDONLY));
    ejsSetProperty(ejs, type, ES_App__outputStream, ejsCreateFileFromFd(ejs, 1, "stdout", O_WRONLY));
    ejsSetProperty(ejs, type, ES_App__errorStream, ejsCreateFileFromFd(ejs, 2, "stderr", O_WRONLY));

    ejsBindMethod(ejs, type, ES_App_args, app_args);
    ejsBindMethod(ejs, type, ES_App_createSearch, app_createSearch);
    ejsBindMethod(ejs, type, ES_App_dir, app_dir);
    ejsBindMethod(ejs, type, ES_App_chdir, app_chdir);
    ejsBindMethod(ejs, type, ES_App_exeDir, app_exeDir);
    ejsBindMethod(ejs, type, ES_App_exePath, app_exePath);
    ejsBindMethod(ejs, type, ES_App_env, app_env);
    ejsBindMethod(ejs, type, ES_App_exit, app_exit);
    ejsBindMethod(ejs, type, ES_App_getenv, app_getenv);
    ejsBindMethod(ejs, type, ES_App_gid, app_gid);
    ejsBindMethod(ejs, type, ES_App_putenv, app_putenv);
    ejsBindMethod(ejs, type, ES_App_pid, app_pid);
    ejsBindMethod(ejs, type, ES_App_run, app_run);
    ejsBindAccess(ejs, type, ES_App_search, app_search, app_set_search);
    ejsBindMethod(ejs, type, ES_App_sleep, app_sleep);
    ejsBindMethod(ejs, type, ES_App_uid, app_uid);
    ejsBindMethod(ejs, type, ES_App_getpass, app_getpass);
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
