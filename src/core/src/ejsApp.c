/*
    ejsApp.c -- App class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*  
    Get the application command line arguments
    static function get args(): String
 */
static EjsObj *app_args(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsArray    *args;
    int         i;

    args = ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, ejsCreateString(ejs, ejs->argv[i]));
    }
    return (EjsObj*) args;
}


/*  
    Get the current working directory
    function get dir(): Path
 */
static EjsObj *app_dir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathAndFree(ejs, mprGetCurrentPath(ejs));
}


/*  
    Set the current working directory
    function chdir(value: String|Path): void
 */
static EjsObj *app_chdir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *path;

    mprAssert(argc == 1);

    if (ejsIsPath(ejs, argv[0])) {
        path = ((EjsPath*) argv[0])->path;
    } else if (ejsIsString(argv[0])) {
        path = ejsGetString(ejs, argv[0]);
    } else {
        ejsThrowIOError(ejs, "Bad path");
        return NULL;
    }

    if (chdir((char*) path) < 0) {
        ejsThrowIOError(ejs, "Can't change the current directory");
    }
    return 0;
}

/*  
    Get an environment var
    function getenv(key: String): String
 */
static EjsObj *app_getenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *value;

    value = getenv(ejsGetString(ejs, argv[0]));
    if (value == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, value);
}


/*  
    Put an environment var
    function putenv(key: String, value: String): void
 */
static EjsObj *app_putenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
#if !WINCE
#if BLD_UNIX_LIKE
    char    *key, *value;

    key = mprStrdup(ejs, ejsGetString(ejs, argv[0]));
    value = mprStrdup(ejs, ejsGetString(ejs, argv[1]));
    setenv(key, value, 1);
#else
    char   *cmd;

    cmd = mprStrcat(app, -1, ejsGetString(ejs, argv[0]), "=", ejsGetString(ejs, argv[1]), NULL);
    putenv(cmd);
#endif
#endif
    return 0;
}


/*  
    Get the directory containing the application's executable file.
    static function get exeDir(): Path
 */
static EjsObj *app_exeDir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePath(ejs, mprGetAppDir(ejs));
}


/*  
    Get the application's executable filename.
    static function get exePath(): Path
 */
static EjsObj *app_exePath(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePath(ejs, mprGetAppPath(ejs));
}


/*  
    Exit the application
    static function exit(status: Number): void
 */
static EjsObj *app_exit(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    int     status;

    status = argc == 0 ? 0 : ejsGetInt(ejs, argv[0]);
    mprBreakpoint();
    //  TODO -- Make more uniform. Zero status won't exit immediately but non-zero will????
    if (status != 0) {
        exit(status);
    } else {
        mprTerminate(mprGetMpr(ejs), 1);
        ejsAttention(ejs);
    }
    return 0;
}


/*  
    Control if the application will exit when the last script completes.
    static function noexit(exit: Boolean): void
 */
static EjsObj *app_noexit(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    ejs->flags |= EJS_FLAG_NOEXIT;
    return 0;
}


#if UNUSED
/*  
    static function name(): String
 */
static EjsObj *app_name(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, mprGetAppName(ejs));
}


/*  
    static function set name(str: String): Void
 */
static EjsObj *app_set_name(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *name;

    name = ejsGetString(argv[0]);
    mprSetAppName(ejs, name, name, NULL);
    return 0;
}
#endif


/*  
    Get the ejs module search path. Does not actually read the environment.
    function get search(): Array
 */
static EjsObj *app_search(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    return (EjsObj*) ejs->search;
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
    function get createSearch(searchPaths: String): Void
 */
static EjsObj *app_createSearch(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *searchPath;

    searchPath = (argc == 0) ? NULL : ejsGetString(ejs, argv[0]);
    return (EjsObj*) ejsCreateSearchPath(ejs, searchPath);
}


/*  
    Need this routine because ejs->exiting must be tested for workers
 */
void ejsServiceEvents(Ejs *ejs, int timeout, int flags)
{
    MprTime     expires;
    int         rc, remaining;

    if (timeout < 0) {
        timeout = MPR_MAX_TIMEOUT;
    }
    expires = mprGetTime(ejs) + timeout;
    remaining = timeout;
    do {
        rc = mprServiceEvents(ejs, ejs->dispatcher, remaining, MPR_SERVICE_ONE_THING);
        if (rc > 0 && flags & MPR_SERVICE_ONE_THING) {
            break;
        }
        remaining = (int) (expires - mprGetTime(ejs));
        if (ejs->exception) {
            ejsClearException(ejs);
        }
    } while (remaining >= 0 && !mprIsExiting(ejs) && !ejs->exiting && !ejs->exception);
}


/*  
    static function eventLoop(timeout: Number = -1, oneEvent: Boolean = false): void
 */
static EjsObj *app_eventLoop(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    int     timeout, oneEvent;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]) : MPR_MAX_TIMEOUT;
    oneEvent = (argc > 1) ? (argv[1] == ejs->trueValue) : 0;
    ejsServiceEvents(ejs, timeout, oneEvent ? MPR_SERVICE_ONE_THING: 0);
    return 0;
}


/*  
    Pause the application
    static function sleep(delay: Number = -1): void
 */
static EjsObj *app_sleep(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    int         timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]): MPR_MAX_TIMEOUT;
    ejsServiceEvents(ejs, timeout, MPR_SERVICE_NAP);
    return 0;
}


/*********************************** Factory **********************************/

void ejsConfigureAppType(Ejs *ejs)
{
    EjsType     *type;

    type = ejs->appType = ejsGetTypeByName(ejs, EJS_EJS_NAMESPACE, "App");
    mprAssert(type);

    ejsSetProperty(ejs, type, ES_App__inputStream, ejsCreateFileFromFd(ejs, 0, "stdin", O_RDONLY));
    ejsSetProperty(ejs, type, ES_App__outputStream, ejsCreateFileFromFd(ejs, 1, "stdout", O_WRONLY));
    ejsSetProperty(ejs, type, ES_App__errorStream, ejsCreateFileFromFd(ejs, 2, "stderr", O_WRONLY));

    ejsBindMethod(ejs, type, ES_App_args, (EjsProc) app_args);
    ejsBindMethod(ejs, type, ES_App_dir, (EjsProc) app_dir);
    ejsBindMethod(ejs, type, ES_App_chdir, (EjsProc) app_chdir);
    ejsBindMethod(ejs, type, ES_App_exeDir, (EjsProc) app_exeDir);
    ejsBindMethod(ejs, type, ES_App_exePath, (EjsProc) app_exePath);
    ejsBindMethod(ejs, type, ES_App_exit, (EjsProc) app_exit);
    ejsBindMethod(ejs, type, ES_App_getenv, (EjsProc) app_getenv);
    ejsBindMethod(ejs, type, ES_App_putenv, (EjsProc) app_putenv);
#if ES_App_name && UNUSED
    ejsBindAccess(ejs, type, ES_App_name, (EjsProc) app_name, (EjsProc) app_set_name);
#endif
    ejsBindMethod(ejs, type, ES_App_noexit, (EjsProc) app_noexit);
    ejsBindMethod(ejs, type, ES_App_createSearch, (EjsProc) app_createSearch);
    ejsBindAccess(ejs, type, ES_App_search, (EjsProc) app_search, (EjsProc) app_set_search);
    ejsBindMethod(ejs, type, ES_App_eventLoop, (EjsProc) app_eventLoop);
    ejsBindMethod(ejs, type, ES_App_sleep, (EjsProc) app_sleep);

#if FUTURE
    (ejs, type, ES_App_permissions, (EjsProc) getPermissions,
        ES_App_set_permissions, (EjsProc) setPermissions);
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
