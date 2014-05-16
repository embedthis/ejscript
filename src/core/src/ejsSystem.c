/*
    ejsSystem.c -- System class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************ Methods *********************************/
/*
    function get hostname(): String
 */
static EjsString *system_hostname(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, mprGetHostName());
}


/*
    function get ipaddr(): String
 */
static EjsString *system_ipaddr(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    //  TODO - move this into MPR and call mprSetIpAddr
#if ME_UNIX_LIKE || ME_WIN_LIKE
    struct addrinfo *res, *reslist, hints;
    cchar           *ip;
    char            ipaddr[ME_MAX_PATH], service[ME_MAX_PATH];
    int             rc;

    if ((ip = mprGetIpAddr(ejs)) != 0) {
        return ejsCreateStringFromAsc(ejs, mprGetIpAddr(ejs));
    }
    memset((char*) &hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    if ((rc = getaddrinfo(mprGetHostName(), NULL, &hints, &reslist)) == 0) {
        ip = 0;
        //  TODO - support IPv6
        for (res = reslist; res; res = res->ai_next) {
            if (getnameinfo(res->ai_addr, (socklen_t) res->ai_addrlen, ipaddr, (int) sizeof(ipaddr) - 1, service, 
                    (int) sizeof(service) - 1, NI_NUMERICHOST | NI_NUMERICSERV | NI_NOFQDN) == 0) {
                if (strncmp(ipaddr, "10.", 3) == 0 || strncmp(ipaddr, "127.", 4) == 0 ||
                     strncmp(ipaddr, "169.", 4) == 0 || strncmp(ipaddr, "172.", 4) == 0 ||
                     strncmp(ipaddr, "192.", 4) == 0) {
                    if (ip == 0) {
                        ip = sclone(ipaddr);
                    }
                } else {
                    ip = sclone(ipaddr);
                    break;
                }
            }
        }
        return ejsCreateStringFromAsc(ejs, ip ? ip : "127.0.0.1");
    } else {
        mprError("Cannot get IP address, check system hostname. Error %d.\n", rc);
    }
#endif
    return ESV(null);
}


#if ES_System_tmpdir
/*
    function get tmpdir(): Path
 */
static EjsPath *system_tmpdir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *dir;

#if WINCE
    dir = "/Temp";
#elif ME_WIN_LIKE
{
    MprFileSystem   *fs;
    fs = mprLookupFileSystem("/");
    dir = sclone(getenv("TEMP"));
#if UNUSED
    mprMapSeparators(dir, defaultSep(fs));
#endif
}
#elif VXWORKS
    dir = ".";
#else
    dir = "/tmp";
#endif
    return ejsCreatePathFromAsc(ejs, dir);
}
#endif

/************************************ Factory *********************************/

PUBLIC void ejsConfigureSystemType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "System"), 0, 0, 0)) == 0) {
        return;
    }
    ejsBindMethod(ejs, type, ES_System_hostname, system_hostname);
    ejsBindMethod(ejs, type, ES_System_ipaddr, system_ipaddr);
#if ES_System_tmpdir
    ejsBindMethod(ejs, type, ES_System_tmpdir, system_tmpdir);
#endif
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
