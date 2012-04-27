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
    return ejsCreateStringFromAsc(ejs, mprGetHostName(ejs));
}


/*
    function get ipaddr(): String
 */
static EjsString *system_ipaddr(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    //  MOB - move this into MPR and call mprSetIpAddr
#if BLD_UNIX_LIKE || BLD_WIN_LIKE
    struct addrinfo *res, *reslist, hints;
    cchar           *ip;
    char            ipaddr[MPR_MAX_STRING], service[MPR_MAX_STRING];

    if ((ip = mprGetIpAddr(ejs)) != 0) {
        return ejsCreateStringFromAsc(ejs, mprGetIpAddr(ejs));
    }
    memset((char*) &hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    if (getaddrinfo(mprGetHostName(ejs), NULL, &hints, &reslist) == 0) {
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
        mprLog(1, "Can't get IP address. Getaddrinfo failed %d\n", errno);
    }
#endif
    return ESV(null);
}


/************************************ Factory *********************************/

void ejsConfigureSystemType(Ejs *ejs)
{
    EjsType         *type;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "System"), 0, 0, 0)) == 0) {
        return;
    }
    ejsBindMethod(ejs, type, ES_System_hostname, system_hostname);
    ejsBindMethod(ejs, type, ES_System_ipaddr, system_ipaddr);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
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
