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

static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn);
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
    sp->async = 1;
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
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
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
    function accept(): Request
 */
static EjsObj *hs_accept(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpConn    *conn;

    if ((conn = httpAcceptConn(sp->server)) == 0) {
        //  MOB -- or should this just be ignored?
        ejsThrowStateError(ejs, "Can't accept connection");
        return 0;
    }
    return (EjsObj*) createRequest(sp, conn);
}


/*  
    function get async(): Boolean
 */
static EjsObj *hs_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (sp->async) ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *hs_set_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->async = ejsGetBoolean(ejs, argv[0]);
    if (sp->server) {
        httpSetServerAsync(sp->server, sp->async);
    }
    return 0;
}


#if UNUSED
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
#endif


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
    function get isSecure(): Void
 */
static EjsObj *hs_isSecure(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return sp->ssl ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/*  
    function listen(endpoint): Void
    An endpoint can be either a "port", "ip:port", or null
 */
static EjsObj *hs_listen(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpServer  *server;
    EjsString   *address;
    EjsObj      *endpoint;
    EjsPath     *root;

    endpoint = argv[0];

    if (sp->server) {
        mprFree(sp->server);
        sp->server = 0;
    }
    if (endpoint == ejs->nullValue) {
        sp->obj.permanent = 1;
        if (ejs->location) {
            ejs->location->context = sp;
        } else {
            ejsThrowStateError(ejs, "Can't find web server context for Ejscript. Check EjsStartup directive");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    if (ejs->location) {
        /* Being called hosted - ignore endpoint value */
        sp->obj.permanent = 1;
        ejs->location->context = sp;
        return (EjsObj*) ejs->nullValue;
    }
    address = ejsToString(ejs, endpoint);
    mprParseIp(ejs, address->value, &sp->ip, &sp->port, 80);

    /*
        The server uses the ejsDispatcher. This is VERY important. All connections will inherit this also.
        This serializes all activity on one dispatcher.
     */
    if ((server = httpCreateServer(ejs->http, sp->ip, sp->port, ejs->dispatcher)) == 0) {
        ejsThrowIOError(ejs, "Can't create server object");
        return 0;
    }
    sp->server = server;
    if (sp->ssl) {
        httpSetServerSsl(server, sp->ssl);
    }
    if (sp->name) {
        httpSetServerName(server, sp->name);
    }
    httpSetServerSoftware(server, EJS_HTTPSERVER_NAME);
    httpSetServerAsync(server, sp->async);

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
    return ejs->nullValue;
}


/*  
    function get name(): String
 */
static EjsObj *hs_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->name) {
        return (EjsObj*) ejsCreateString(ejs, sp->name);
    }
    return (EjsObj*) ejs->nullValue;
}


/*  
    function set name(hostname: String): Void
 */
static EjsObj *hs_set_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    mprFree(sp->name);
    sp->name = mprStrdup(sp, ejsGetString(ejs, argv[0]));
    if (sp->server) {
        httpSetServerName(sp->server, sp->name);
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
    ejsRemoveObserver(ejs, sp->emitter, argv[0], argv[1]);
    return 0;
}
#endif


/*  
    function secure(keyFile: Path, certFile: Path!, protocols: Array = null, ciphers: Array = null): Void
 */
static EjsObj *hs_secure(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
#if BLD_FEATURE_SSL
    EjsArray    *protocols;
    cchar       *token;
    int         mask, protoMask, i;

    if (sp->ssl == 0 && ((sp->ssl = mprCreateSsl(sp)) == 0)) {
        return 0;
    }
    if (httpLoadSsl(ejs->http) < 0) {
        ejsThrowStateError(ejs, "Can't load SSL provider");
        return 0;
    }
    if (argv[0] != ejs->nullValue) {
        mprSetSslKeyFile(sp->ssl, ejsGetString(ejs, argv[0]));
    }
    if (argv[1] != ejs->nullValue) {
        mprSetSslCertFile(sp->ssl, ejsGetString(ejs, argv[1]));
    }

    if (argc >= 3 && ejsIsArray(argv[2])) {
        protocols = (EjsArray*) argv[2];
        protoMask = 0;
        for (i = 0; i < protocols->length; i++) {
            token = ejsGetString(ejs, ejsToString(ejs, ejsGetProperty(ejs, protocols, i)));
            mask = -1;
            if (*token == '-') {
                token++;
                mask = 0;
            } else if (*token == '+') {
                token++;
            }
            if (mprStrcmpAnyCase(token, "SSLv2") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV2 & ~mask);
                protoMask |= (MPR_PROTO_SSLV2 & mask);

            } else if (mprStrcmpAnyCase(token, "SSLv3") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV3 & ~mask);
                protoMask |= (MPR_PROTO_SSLV3 & mask);

            } else if (mprStrcmpAnyCase(token, "TLSv1") == 0) {
                protoMask &= ~(MPR_PROTO_TLSV1 & ~mask);
                protoMask |= (MPR_PROTO_TLSV1 & mask);

            } else if (mprStrcmpAnyCase(token, "ALL") == 0) {
                protoMask &= ~(MPR_PROTO_ALL & ~mask);
                protoMask |= (MPR_PROTO_ALL & mask);
            }
        }
        mprSetSslProtocols(sp->ssl, protoMask);
    }
    if (argc >= 4 && ejsIsArray(argv[3])) {
        mprSetSslCiphers(sp->ssl, ejsGetString(ejs, argv[3]));
    }
    mprConfigureSsl(sp->ssl);
#else
    ejsThrowReferenceError(ejs, "SSL support was not included in the build");
#endif
    return 0;
}


/*  
    function get software(headers: Object = null): Void
 */
static EjsObj *hs_software(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, EJS_HTTPSERVER_NAME);
}


/*  
    function verifyClients(caCertPath: Path, caCertFile: Path): Void
 */
static EjsObj *hs_verifyClients(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  TODO
    return 0;
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
        httpPutForService(q, packet, 0);
        if (rec->form) {
            httpAddVarsFromQueue(q);
        }
        HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);

    } else if (trans->writeComplete) {
        httpFreePacket(q, packet);

    } else {
        httpJoinPacketForService(q, packet, 0);
        HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
    }
}


static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn)
{
    Ejs             *ejs;
    EjsRequest      *req;
    EjsPath         *dirPath;
    HttpLocation    *location;
    cchar           *dir;

    if (conn->transmitter->handler->match) {
        /*
            Hosted handler. Must supply a location block which defines the HttpServer instance.
         */
        location = conn->receiver->location;
        if (location == 0 || location->context == 0) {
            mprError(sp, "Location block is not defined for request");
            return 0;
        }
        sp = (EjsHttpServer*) location->context;
        ejs = sp->ejs;
        dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
        dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->path : conn->documentRoot;
        if (sp->server == 0) {
            sp->server = conn->server;
            sp->server->ssl = location->ssl;
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
    return req;
}


static void runEjs(HttpQueue *q)
{
    EjsHttpServer   *sp;
    EjsRequest      *req;
    HttpConn        *conn;

    conn = q->conn;
    if (!conn->error) {
        sp = httpGetServerContext(conn->server);
        if ((req = httpGetConnContext(conn)) == 0) {
            if ((req = createRequest(sp, conn)) == 0) {
                return;
            }
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
    //  MOB -- need destructor to shutdown properly

    prototype = type->prototype;
    ejsBindConstructor(ejs, type, (EjsProc) hs_HttpServer);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_accept, (EjsProc) hs_accept);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, (EjsProc) hs_address);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, (EjsProc) hs_async, (EjsProc) hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, (EjsProc) hs_close);
#if ES_ejs_web_HttpServer_isSecure
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_isSecure, (EjsProc) hs_isSecure);
#endif
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, (EjsProc) hs_listen);
#if ES_ejs_web_HttpServer_name
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_name, (EjsProc) hs_name, (EjsProc) hs_set_name);
#endif
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, (EjsProc) hs_port);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_observe, (EjsProc) hs_observe);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_removeObserver, (EjsProc) hs_removeObserver);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_secure, (EjsProc) hs_secure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_verifyClients, (EjsProc) hs_verifyClients);
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
