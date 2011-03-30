/*
    ejsWeb.c -- Ejscript web framework.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"
#include    "ecCompiler.h"
#include    "ejs.web.slots.h"

/***************************** Forward Declarations ***************************/

static int configureWebTypes(Ejs *ejs);

/************************************ Code ************************************/

static int requestWorker(EjsRequest *req, MprEvent *event)
{
    Ejs         *ejs;
    EjsObj      *argv[2];

    ejs = req->ejs;
    mprAssert(ejs);
    mprAssert(req->app);
    
    argv[0] = (EjsObj*) req->app;
    argv[1] = (EjsObj*) req;
    ejsRunFunctionBySlot(ejs, S(Web), ES_ejs_web_Web_workerHelper, 2, argv);
    //  MOB - does this need to send a readable event / NOTIFY ... READABLE
    return 0; 
}


/*
    static function worker(app: Function, req: Request): Void
 */
static EjsObj *req_worker(Ejs *ejs, EjsObj *web, int argc, EjsObj **argv)
{
    Ejs         *nejs;
    EjsRequest  *req, *nreq;
    EjsObj      *app;
    HttpConn    *conn;

    app = argv[0];
    req = (EjsRequest*) argv[1];
    if ((nejs = ejsCreate(0, 0, 0, 0, 0, 0)) == 0) {
        ejsThrowStateError(ejs, "Can't create interpreter to service request");
        return 0;
    }
    conn = req->conn;
    conn->mark = nejs;
    conn->newDispatcher = nejs->dispatcher;
    
    nejs->loc = ejs->loc;
    if (ejsLoadModule(nejs, ejsCreateStringFromAsc(nejs, "ejs.web"), -1, -1, EJS_LOADER_RELOAD) < 0) {
        ejsThrowStateError(ejs, "Can't load ejs.web.mod: %s", ejsGetErrorMsg(nejs, 1));
        return 0;
    }
    //  MOB -- not really doing a clone. This is a minimal copy. Should rename perhaps?
    if ((nreq = ejsCloneRequest(nejs, req, 1)) == 0) {
        ejsThrowStateError(ejs, "Can't clone request");
        return 0;
    }
    httpSetConnContext(conn, nreq);
    nreq->app = app;

    //  MOB -- not really doing a clone. This is a minimal copy. Should rename perhaps?
    if ((nreq->server = ejsCloneHttpServer(nejs, req->server, 1)) == 0) {
        ejsThrowStateError(ejs, "Can't clone request");
        return 0;
    }
    conn->workerEvent = mprCreateEvent(conn->dispatcher, "RequestWorker", 0, requestWorker, nreq, MPR_EVENT_DONT_QUEUE);
    if (conn->workerEvent == 0) {
        ejsThrowStateError(ejs, "Can't create worker event");
    }
    return 0;
}


/*  
    HTML escape a string
    function escapeHtml(str: String): String
 */
static EjsObj *web_escapeHtml(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsString   *str;

    str = (EjsString*) argv[0];
    return (EjsObj*) ejsCreateStringFromAsc(ejs, mprEscapeHtml(str->value));
}


/******************************************************************************/

static int configureWebTypes(Ejs *ejs)
{
    EjsType     *type;
    int         slotNum;

    if ((type = ejsGetTypeByName(ejs, N("ejs.web", "Web"))) == 0) {
        mprError("Can't find ejs.web::Web class");
        ejs->hasError = 1;
        return MPR_ERR_CANT_INITIALIZE;
    }
    ejsSetSpecialType(ejs, S_Web, type);

    ejsBindMethod(ejs, type, ES_ejs_web_Web_worker, req_worker);

    if ((slotNum = ejsLookupProperty(ejs, ejs->global, N("ejs.web", "escapeHtml"))) != 0) {
        ejsBindFunction(ejs, ejs->global, slotNum, web_escapeHtml);
    }
    ejsConfigureHttpServerType(ejs);
    ejsConfigureRequestType(ejs);
    ejsConfigureSessionType(ejs);
    return 0;
}


/*  
    Module load entry point. This must be idempotent as it will be called for each new interpreter created.
 */
int ejs_web_Init(Ejs *ejs, MprModule *mp)
{
    return ejsAddNativeModule(ejs, "ejs.web", configureWebTypes, _ES_CHECKSUM_ejs_web, EJS_LOADER_ETERNAL);
}


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
