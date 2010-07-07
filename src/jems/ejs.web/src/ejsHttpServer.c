/*
    ejsHttpServer.c -- Ejscript Http Server.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"
#include    "ecCompiler.h"
#include    "ejs.web.slots.h"

/********************************** Forwards **********************************/

static void stateChangeNotifier(HttpConn *conn, int state, int notifyFlags);

/************************************ Code ************************************/
/*  
    function HttpServer(serverRoot: Path = ".", documentRoot: Path = ".")
    Constructor function
 */
static EjsObj *hs_HttpServer(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    EjsObj      *serverRoot, *documentRoot;

    sp->ejs = ejs;
    serverRoot = (argc >= 1) ? argv[0] : (EjsObj*) ejsCreatePath(ejs, ".");
    ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_serverRoot, serverRoot);

    documentRoot = (argc >= 1) ? argv[1] : (EjsObj*) ejsCreatePath(ejs, ".");
    ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_documentRoot, documentRoot);
    return (EjsObj*) sp;
}


/*  
    function observe(name: [String|Array], listener: Function): Void
 */
static EjsObj *hs_observe(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  TODO -- should fire if currently readable / writable (also socket etc)
    ejsAddListener(ejs, &sp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function get address(): Void
 */
static EjsObj *hs_address(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, sp->ip);
}


/*  
    function get async(): Boolean
 */
static EjsObj *hs_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return httpGetServerAsync(sp->server) ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *hs_set_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    httpSetServerAsync(sp->server, (argv[0] == (EjsObj*) ejs->trueValue));
    return 0;
}


/*  
    function attach(): Void
 */
static EjsObj *hs_attach(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  MOB -- would be great if users did not have to call attach
    sp->obj.permanent = 1;
    if (ejs->location) {
        ejs->location->context = sp;
    } else {
        ejsThrowStateError(ejs, "attach can only be called when hosted inside a supported web server");
    }
    return 0;
}


/*  
    function close(): Void
 */
static EjsObj *hs_close(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->server) {
        //  MOB -- who make sure that the server object is permanent?
        ejsSendEvent(ejs, sp->emitter, "close", (EjsObj*) sp);
        mprFree(sp->server);
        sp->server = 0;
    }
    return 0;
}


/*  
    function listen(address): Void
    Address Can be either an "ip", "ip:port" or port
 */
static EjsObj *hs_listen(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpServer  *server;
    EjsString   *address;
    EjsPath     *root;

    if (ejs->location) {
        /* Being called hosted */
        sp->obj.permanent = 1;
        ejs->location->context = sp;
        return 0;
    }

    if (sp->server) {
        mprFree(sp->server);
        sp->server = 0;
    }
    address = (EjsString*) argv[0];
    mprParseIp(ejs, address->value, &sp->ip, &sp->port, 80);

    /*
        The server uses the ejsDispatcher. This is VERY important. All connections will inherit this also.
        This serializes all activity on the one dispatcher.
     */
    if ((server = httpCreateServer(ejs->http, sp->ip, sp->port, ejs->dispatcher)) == 0) {
        ejsThrowIOError(ejs, "Can't create server object");
        return 0;
    }
    sp->server = server;

    root = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
    if (ejsIsPath(ejs, root)) {
        //  MOB -- why is this needed? remove?
        server->documentRoot = mprStrdup(server, root->path);
    }

    //  MOB -- is this needed? -- remove
    root = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_serverRoot);
    if (ejsIsPath(ejs, root)) {
        server->serverRoot = mprStrdup(server, root->path);
    }

    //  MOB -- who make sure that the sp object is permanent?

    httpSetServerContext(server, sp);
    httpSetServerNotifier(server, (HttpNotifier) stateChangeNotifier);
    if (httpStartServer(server) < 0) {
        ejsThrowIOError(ejs, "Can't listen on %s", address->value);
        mprFree(sp->server);
        sp->server = 0;
        return 0;
    }
    return 0;
}


/*  
    function get port(): Number
 */
static EjsObj *hs_port(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, sp->port);
}


#if ES_ejs_web_HttpServer_removeObserver
/*  
    function removeObserver(name: [String|Array], listener: Function): Void
 */
static EjsObj *hs_removeObserver(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    ejsRemoveListener(ejs, sp->emitter, argv[0], argv[1]);
    return 0;
}
#endif


/*  
    function get software(headers: Object = null): Void
 */
static EjsObj *hs_software(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, HTTP_NAME);
}


/************************************ Helpers *************************************/
/*
    Notification callback. This routine is called from the Http pipeline on connection state changes. 
    Readable/writable events come with state == 0 and notifyFlags set accordingly.
 */
static void stateChangeNotifier(HttpConn *conn, int state, int notifyFlags)
{
    Ejs             *ejs;
    EjsRequest      *req;

    mprAssert(conn);

    ejs = 0;
    if ((req = httpGetConnContext(conn)) != 0) {
        ejs = req->ejs;
    }

    switch (state) {
    case HTTP_STATE_PARSED:
        conn->transmitter->handler = conn->http->ejsHandler;
        break;

    case HTTP_STATE_ERROR:
        if (req && req->emitter) {
            ejsSendEvent(ejs, req->emitter, "error", (EjsObj*) req);
        }
        break;

    case HTTP_STATE_COMPLETE:
        if (req) {
            if (req->emitter) {
                ejsSendEvent(ejs, req->emitter, "close", (EjsObj*) req);
            }
        }
        break;

    case 0:
        /*  IO event notification for the request.  */
        if (req && req->emitter) {
            if (notifyFlags & HTTP_NOTIFY_READABLE) {
                ejsSendEvent(ejs, req->emitter, "readable", (EjsObj*) req);
            } 
            if (notifyFlags & HTTP_NOTIFY_WRITABLE) {
                ejsSendEvent(ejs, req->emitter, "writable", (EjsObj*) req);
            }
        }
        break;
    }
}


static void closeEjs(HttpQueue *q)
{
    httpSetConnContext(q->conn, 0);
    httpSetRequestNotifier(q->conn, 0);
}


static void incomingEjsData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTransmitter *trans;
    HttpReceiver    *rec;

    conn = q->conn;
    trans = conn->transmitter;
    rec = conn->receiver;

    if (httpGetPacketLength(packet) == 0) {
        if (rec->remainingContent > 0) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient body data");
        }
        if (rec->form) {
            httpAddVarsFromQueue(q);
        }
    } else if (trans->writeComplete) {
        httpFreePacket(q, packet);
    } else {
        httpJoinPacketForService(q, packet, 0);
    }
}


static void runEjs(HttpQueue *q)
{
    EjsHttpServer   *sp;
    EjsRequest      *req;
    EjsPath         *dirPath;
    Ejs             *ejs;
    HttpConn        *conn;
    HttpLocation    *location;
    cchar           *dir;

    conn = q->conn;
    if (!conn->error) {
        sp = httpGetServerContext(conn->server);
        if ((req = httpGetConnContext(conn)) == 0) {
            if (conn->transmitter->handler->match) {
                /*
                    Hosted handler. Must supply a location block which defines the HttpServer instance.
                 */
                location = conn->receiver->location;
                if (location == 0 || location->context == 0) {
                    mprError(q, "Location block is not defined for request");
                    return;
                }
                sp = (EjsHttpServer*) location->context;
                ejs = sp->ejs;
                dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
                dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->path : conn->documentRoot;
                if (sp->server == 0) {
                    sp->server = conn->server;
                    sp->ip = mprStrdup(sp, conn->server->ip);
                    sp->port = conn->server->port;
                    sp->dir = mprStrdup(sp, dir);
                }
                httpSetServerContext(conn->server, sp);
                httpSetRequestNotifier(conn, (HttpNotifier) stateChangeNotifier);
            } else {
                ejs = sp->ejs;
                dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
                dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->path : ".";
            }
            req = ejsCreateRequest(ejs, sp, conn, dir);
            httpSetConnContext(conn, req);
            conn->dispatcher = ejs->dispatcher;
            conn->documentRoot = conn->server->documentRoot;
            conn->transmitter->handler = ejs->http->ejsHandler;
        }
        ejsSendEvent(sp->ejs, sp->emitter, "readable", (EjsObj*) req);
    }
}


static void startEjs(HttpQueue *q)
{
    HttpReceiver    *rec;

    rec = q->conn->receiver;
    if (!rec->form && !rec->upload) {
        runEjs(q);
    }
}


static void processEjs(HttpQueue *q)
{
    HttpReceiver    *rec;

    rec = q->conn->receiver;
    if (rec->form || rec->upload) {
        runEjs(q);
    }
}


/* 
    One time initialization of the http pipeline handler
 */
HttpStage *ejsAddWebHandler(Http *http)
{
    HttpStage   *handler;

    mprAssert(http);

    if (http->ejsHandler) {
        return http->ejsHandler;
    }
    handler = httpCreateHandler(http, "ejsHandler",
        HTTP_STAGE_GET | HTTP_STAGE_HEAD | HTTP_STAGE_POST | HTTP_STAGE_PUT | HTTP_STAGE_VARS);
    if (handler == 0) {
        return 0;
    }
    http->ejsHandler = handler;
    handler->close = closeEjs;
    handler->incomingData = incomingEjsData;
    handler->start = startEjs;
    handler->process = processEjs;
    return handler;
}


/*  
    Mark the object properties for the garbage collector
 */
static void markHttpServer(Ejs *ejs, EjsHttpServer *sp)
{
    //  MOB -- not needed
    ejsMarkObject(ejs, (EjsObj*) sp);

    if (sp->emitter) {
        ejsMark(ejs, (EjsObj*) sp->emitter);
    }
}


void ejsConfigureHttpServerType(Ejs *ejs)
{
    EjsObj      *prototype;
    EjsType     *type;

    type = ejsConfigureNativeType(ejs, "ejs.web", "HttpServer", sizeof(EjsHttpServer));
    type->helpers.mark = (EjsMarkHelper) markHttpServer;

    prototype = type->prototype;
    ejsBindConstructor(ejs, type, (EjsProc) hs_HttpServer);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_observe, (EjsProc) hs_observe);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, (EjsProc) hs_address);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_attach, (EjsProc) hs_attach);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, (EjsProc) hs_async, (EjsProc) hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, (EjsProc) hs_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, (EjsProc) hs_listen);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, (EjsProc) hs_port);
#if ES_ejs_web_HttpServer_removeObserver
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_removeObserver, (EjsProc) hs_removeObserver);
#endif
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_software, (EjsProc) hs_software);
    
    ejsAddWebHandler(ejs->http);
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
