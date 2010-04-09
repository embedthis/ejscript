/*
    ejsSystem.c -- System class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    function run(cmd: String): String
 */
static EjsObj *system_run(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    EjsString   *result;
    cchar       *cmdline;
    char        *err, *output;
    int         status;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    cmd = mprCreateCmd(ejs, ejs->dispatcher);
    cmdline = ejsGetString(ejs, argv[0]);
    status = mprRunCmd(cmd, cmdline, &output, &err, 0);
    if (status) {
        ejsThrowError(ejs, "Command failed: %s\n\nExit status: %d\n\nError Output: \n%s\nPrevious Output: \n%s\n", 
            cmdline, status, err, output);
        mprFree(cmd);
        return 0;
    }
    result = ejsCreateString(ejs, output);
    mprFree(cmd);
    return (EjsObj*) result;
}


/*
    function runx(cmd: String): Void
 */
static EjsObj *system_runx(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    char        *err;
    int         status;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    cmd = mprCreateCmd(ejs, ejs->dispatcher);
    status = mprRunCmd(cmd, ejsGetString(ejs, argv[0]), NULL, &err, 0);
    if (status) {
        ejsThrowError(ejs, "Can't run command: %s\nDetails: %s", ejsGetString(ejs, argv[0]), err);
        mprFree(err);
    }
    mprFree(cmd);
    return 0;
}


//  TODO - refactor and rename
/*
    function daemon(cmd: String): Number
 */
static EjsObj *system_daemon(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprCmd      *cmd;
    int         status, pid;

    mprAssert(argc == 1 && ejsIsString(argv[0]));

    cmd = mprCreateCmd(ejs, ejs->dispatcher);
    status = mprRunCmd(cmd, ejsGetString(ejs, argv[0]), NULL, NULL, MPR_CMD_DETACH);
    if (status) {
        ejsThrowError(ejs, "Can't run command: %s", ejsGetString(ejs, argv[0]));
    }
    pid = cmd->pid;
    mprFree(cmd);
    return (EjsObj*) ejsCreateNumber(ejs, pid);
}


//  TODO - refactor and rename
/*
    function exec(cmd: String): Void
 */
static EjsObj *system_exec(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if BLD_UNIX_LIKE
    char    **argVector;
    int     argCount;

    mprMakeArgv(ejs, NULL, ejsGetString(ejs, argv[0]), &argCount, &argVector);
    execv(argVector[0], argVector);
#endif
    ejsThrowStateError(ejs, "Can't exec %s", ejsGetString(ejs, argv[0]));
    return 0;
}


/*
    function get hostname(): String
 */
static EjsObj *system_hostname(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringAndFree(ejs, mprStrdup(ejs, mprGetHostName(ejs)));
}


/*
    function get ipaddr(): String
 */
static EjsObj *system_ipaddr(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
#if BLD_UNIX_LIKE || BLD_WIN_LIKE
    struct addrinfo *res, *reslist, hints;
    cchar           *ip;
    char			ipaddr[MPR_MAX_STRING], service[MPR_MAX_STRING];

    if ((ip = mprGetIpAddr(ejs)) != 0) {
        return (EjsObj*) ejsCreateString(ejs, mprGetIpAddr(ejs));
    }
    memset((char*) &hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    if (getaddrinfo(mprGetHostName(ejs), NULL, &hints, &reslist) == 0) {
        ip = 0;
        //  TODO - support IPv6
        for (res = reslist; res; res = res->ai_next) {
            if (getnameinfo(res->ai_addr, res->ai_addrlen, ipaddr, sizeof(ipaddr) - 1, service, sizeof(service) - 1, 
                    NI_NUMERICHOST | NI_NUMERICSERV | NI_NOFQDN) == 0) {
                if (strncmp(ipaddr, "10.", 3) == 0 || strncmp(ipaddr, "127.", 4) == 0 ||
                     strncmp(ipaddr, "169.", 4) == 0 || strncmp(ipaddr, "172.", 4) == 0 ||
                     strncmp(ipaddr, "192.", 4) == 0) {
                    if (ip == 0) {
                        ip = mprStrdup(ejs, ipaddr);
                    }
                } else {
                    ip = mprStrdup(ejs, ipaddr);
                    break;
                }
            }
        }
        return (EjsVar*) ejsCreateString(ejs, ip ? ip : "127.0.0.1");
    }
#endif
    return ejs->nullValue;
}


/************************************ Factory *********************************/

void ejsConfigureSystemType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsGetTypeByName(ejs, EJS_EJS_NAMESPACE, "System")) == 0) {
        mprError(ejs, "Can't find System type");
        return;
    }
    ejsBindMethod(ejs, type, ES_System_daemon, (EjsProc) system_daemon);
    ejsBindMethod(ejs, type, ES_System_exec, (EjsProc) system_exec);
    ejsBindMethod(ejs, type, ES_System_run, (EjsProc) system_run);
    ejsBindMethod(ejs, type, ES_System_runx, (EjsProc) system_runx);
    ejsBindMethod(ejs, type, ES_System_hostname, (EjsProc) system_hostname);
#if ES_System_ipaddr
    ejsBindMethod(ejs, type, ES_System_ipaddr, (EjsProc) system_ipaddr);
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
