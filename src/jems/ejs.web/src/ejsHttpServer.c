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
static EjsHttpServer *getServerContext(HttpConn *conn);
static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp);
static void setupConnTrace(HttpConn *conn);
static void stateChangeNotifier(HttpConn *conn, int state, int notifyFlags);

/************************************ Code ************************************/
/*  
    function get address(): String
 */
static EjsObj *hs_address(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->ip) {
        return (EjsObj*) ejsCreateStringFromAsc(ejs, sp->ip);
    } 
    return ejs->nullValue;
}


/*  
    function accept(): Request
 */
static EjsObj *hs_accept(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    MprEvent    event;

    memset(&event, 0, sizeof(MprEvent));
    event.dispatcher = sp->server->dispatcher;
    if ((conn = httpAcceptConn(sp->server, &event)) == 0) {
        /* Just ignore */
        mprError("Can't accept connection");
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


/*  
    function close(): Void
 */
static EjsObj *hs_close(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->server) {
        ejsSendEvent(ejs, sp->emitter, "close", NULL, sp);
        httpDestroyServer(sp->server);
#if MOB
        ejsStopSessionTimer(sp);
#endif
        sp->server = 0;
        mprRemoveRoot(sp);
    }
    return 0;
}


/*  
    function get limits(): Object
 */
static EjsObj *hs_limits(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpLimits  *limits;

    if (sp->limits == 0) {
        sp->limits = ejsCreateEmptyPot(ejs);
        limits = (sp->server) ? sp->server->limits : ejs->http->serverLimits;
        mprAssert(limits);
        ejsGetHttpLimits(ejs, sp->limits, limits, 1);
    }
    return sp->limits;
}


/*  
    function on(name: [String|Array], observer: Function): Void
 */
static EjsObj *hs_on(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  TODO -- should fire if currently readable / writable (also socket etc)
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function setLimits(limits: Object): Void
 */
static EjsObj *hs_setLimits(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpLimits  *limits;

    if (sp->limits == 0) {
        sp->limits = ejsCreateEmptyPot(ejs);
        limits = (sp->server) ? sp->server->limits : ejs->http->serverLimits;
        mprAssert(limits);
        ejsGetHttpLimits(ejs, sp->limits, limits, 1);
    }
    ejsBlendObject(ejs, sp->limits, argv[0], 1);
    if (sp->server) {
        limits = (sp->server) ? sp->server->limits : ejs->http->serverLimits;
        ejsSetHttpLimits(ejs, limits, sp->limits, 1);
        ejsUpdateSessionLimits(ejs, sp);
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
    HttpHost    *host;
    EjsString   *address;
    EjsObj      *endpoint;
    EjsPath     *root;

    endpoint = (argc >= 1) ? argv[0] : ejs->nullValue;

    if (sp->server) {
        httpDestroyServer(sp->server);
        sp->server = 0;
    }
    if (endpoint == ejs->nullValue) {
        mprAddRoot(sp);
        if (ejs->loc) {
            ejs->loc->context = sp;
        } else {
            ejsThrowStateError(ejs, "Can't find web server context for Ejscript. Check EjsStartup directive");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    if (ejs->loc) {
        mprAddRoot(sp);
        /* Being called hosted - ignore endpoint value */
        ejs->loc->context = sp;
        return (EjsObj*) ejs->nullValue;
    }
    address = ejsToString(ejs, endpoint);
    mprParseIp(address->value, &sp->ip, &sp->port, 80);

    /*
        The server uses the ejsDispatcher. This is VERY important. All connections will inherit this also.
        This serializes all activity on one dispatcher.
     */
    if ((server = httpCreateServer(sp->ip, sp->port, ejs->dispatcher, HTTP_CREATE_HOST)) == 0) {
        ejsThrowIOError(ejs, "Can't create Http server object");
        return 0;
    }
    sp->server = server;
    if (sp->limits) {
        ejsSetHttpLimits(ejs, server->limits, sp->limits, 1);
    }
    if (sp->incomingStages || sp->outgoingStages || sp->connector) {
        setHttpPipeline(ejs, sp);
    }
    if (sp->ssl) {
        httpSecureServer(server->ip, sp->port, sp->ssl);
    }
    if (sp->name) {
        httpSetServerName(server, sp->name);
    }
    httpSetSoftware(server->http, EJS_HTTPSERVER_NAME);
    httpSetServerAsync(server, sp->async);
    httpSetServerContext(server, sp);
    httpSetServerNotifier(server, (HttpNotifier) stateChangeNotifier);

    /*
        This is only required for when http is using non-ejs handlers and/or filters
     */
    host = mprGetFirstItem(server->hosts);
    root = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
    if (ejsIsPath(ejs, root)) {
        httpSetHostDocumentRoot(host, root->value);
    }
    root = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_serverRoot);
    if (ejsIsPath(ejs, root)) {
        httpSetHostServerRoot(host, root->value);
    }

    if (httpStartServer(server) < 0) {
        ejsThrowIOError(ejs, "Can't listen on %s", address->value);
        httpDestroyServer(sp->server);
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
        return (EjsObj*) ejsCreateStringFromAsc(ejs, sp->name);
    }
    return (EjsObj*) ejs->nullValue;
}


/*  
    function set name(hostname: String): Void
 */
static EjsObj *hs_set_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->name = ejsToMulti(ejs, argv[0]);
    if (sp->server) {
        httpSetServerName(sp->server, sp->name);
    }
    return 0;
}


#if ES_ejs_web_HttpServer_off
/*  
    function off(name: [String|Array], observer: Function): Void
 */
static EjsObj *hs_off(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    ejsRemoveObserver(ejs, sp->emitter, argv[0], argv[1]);
    return 0;
}
#endif


/*  
    function get port(): Number
 */
static EjsObj *hs_port(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, sp->port);
}


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
        mprSetSslKeyFile(sp->ssl, ejsToMulti(ejs, argv[0]));
    }
    if (argv[1] != ejs->nullValue) {
        mprSetSslCertFile(sp->ssl, ejsToMulti(ejs, argv[1]));
    }

    if (argc >= 3 && ejsIsArray(ejs, argv[2])) {
        protocols = (EjsArray*) argv[2];
        protoMask = 0;
        for (i = 0; i < protocols->length; i++) {
            token = ejsToMulti(ejs, ejsGetProperty(ejs, protocols, i));
            mask = -1;
            if (*token == '-') {
                token++;
                mask = 0;
            } else if (*token == '+') {
                token++;
            }
            if (scasecmp(token, "SSLv2") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV2 & ~mask);
                protoMask |= (MPR_PROTO_SSLV2 & mask);

            } else if (scasecmp(token, "SSLv3") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV3 & ~mask);
                protoMask |= (MPR_PROTO_SSLV3 & mask);

            } else if (scasecmp(token, "TLSv1") == 0) {
                protoMask &= ~(MPR_PROTO_TLSV1 & ~mask);
                protoMask |= (MPR_PROTO_TLSV1 & mask);

            } else if (scasecmp(token, "ALL") == 0) {
                protoMask &= ~(MPR_PROTO_ALL & ~mask);
                protoMask |= (MPR_PROTO_ALL & mask);
            }
        }
        mprSetSslProtocols(sp->ssl, protoMask);
    }
    if (argc >= 4 && ejsIsArray(ejs, argv[3])) {
        mprSetSslCiphers(sp->ssl, ejsToMulti(ejs, argv[3]));
    }
    mprConfigureSsl(sp->ssl);
#else
    ejsThrowReferenceError(ejs, "SSL support was not included in the build");
#endif
    return 0;
}


/*  
    function setPipeline(incoming: Array, outgoing: Array, connector: String): Void
 */
static EjsObj *hs_setPipeline(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->incomingStages = (EjsArray*) argv[0];
    sp->outgoingStages = (EjsArray*) argv[1];
    sp->connector = ejsToMulti(ejs, argv[2]);

    if (sp->server) {
        /* NOTE: this will only impact future requests */
        setHttpPipeline(ejs, sp);
    }
    return 0;
}


/*  
    function trace(options): Void
 */
static EjsObj *hs_trace(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    ejsSetupTrace(ejs, sp->trace, argv[0]);
    return 0;
}


/*  
    function get software(headers: Object = null): Void
 */
static EjsObj *hs_software(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateStringFromAsc(ejs, EJS_HTTPSERVER_NAME);
}


/*  
    function verifyClients(caCertPath: Path, caCertFile: Path): Void
 */
static EjsObj *hs_verifyClients(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  TODO
    return 0;
}


/************************************ Support *************************************/

//  MOB - rethink this. This should really go into the HttpHost object

static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp) 
{
    EjsString       *vs;
    HttpLoc         *loc;
    Http            *http;
    HttpStage       *stage;
    cchar           *name;
    int             i;

    mprAssert(sp->server);
    http = sp->server->http;
    loc = sp->server->loc;

    if (sp->outgoingStages) {
        httpClearStages(loc, HTTP_STAGE_OUTGOING);
        for (i = 0; i < sp->outgoingStages->length; i++) {
            vs = ejsGetProperty(ejs, sp->outgoingStages, i);
            if (vs && ejsIsString(ejs, vs)) {
                name = vs->value;
                if ((stage = httpLookupStage(http, name)) == 0) {
                    ejsThrowArgError(ejs, "Can't find pipeline stage name %s", name);
                    return;
                }
                httpAddFilter(loc, name, NULL, HTTP_STAGE_OUTGOING);
            }
        }
    }
    if (sp->incomingStages) {
        httpClearStages(loc, HTTP_STAGE_INCOMING);
        for (i = 0; i < sp->incomingStages->length; i++) {
            vs = ejsGetProperty(ejs, sp->incomingStages, i);
            if (vs && ejsIsString(ejs, vs)) {
                name = vs->value;
                if ((stage = httpLookupStage(http, name)) == 0) {
                    ejsThrowArgError(ejs, "Can't find pipeline stage name %s", name);
                    return;
                }
                httpAddFilter(loc, name, NULL, HTTP_STAGE_INCOMING);
            }
        }
    }
    if (sp->connector) {
        if ((stage = httpLookupStage(http, sp->connector)) == 0) {
            ejsThrowArgError(ejs, "Can't find pipeline stage name %s", sp->connector);
            return;
        }
        loc->connector = stage;
    }
}


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
    case HTTP_STATE_BEGIN:
        setupConnTrace(conn);
        break;

    case HTTP_STATE_FIRST:
        if (!(conn->rx->flags & (HTTP_OPTIONS | HTTP_TRACE))) {
            conn->tx->handler = (conn->error) ? conn->http->passHandler : conn->http->ejsHandler;
        }
        break;

    case HTTP_STATE_COMPLETE:
        if (req) {
            if (conn->error) {
                ejsSendRequestErrorEvent(ejs, req);
            }
            ejsSendRequestCloseEvent(ejs, req);
        }
        break;

    case -1:
        if (req) {
            req->conn = 0;
        }
        break;

    case 0:
        /*  IO event notification for the request.  */
        if (req && req->emitter) {
            if (notifyFlags & HTTP_NOTIFY_READABLE) {
                ejsSendEvent(ejs, req->emitter, "readable", NULL, req);
            } 
            if (notifyFlags & HTTP_NOTIFY_WRITABLE) {
                ejsSendEvent(ejs, req->emitter, "writable", NULL, req);
            }
        }
        break;
    }
}


static void closeEjs(HttpQueue *q)
{
    EjsRequest  *req;

    if ((req = httpGetConnContext(q->conn)) != 0) {
        ejsSendRequestCloseEvent(req->ejs, req);
        req->conn = 0;
    }
    httpSetConnContext(q->conn, 0);
    httpSetRequestNotifier(q->conn, 0);
}


static void incomingEjsData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpTx          *trans;
    HttpRx          *rx;

    conn = q->conn;
    trans = conn->tx;
    rx = conn->rx;

    if (httpGetPacketLength(packet) == 0) {
        if (rx->remainingContent > 0) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient body data");
        }
        httpPutForService(q, packet, 0);
        if (rx->form || rx->upload) {
            rx->formVars = httpAddVarsFromQueue(rx->formVars, q);
        }
    } else {
        httpJoinPacketForService(q, packet, 0);
    }
    HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
}


static void setupConnTrace(HttpConn *conn)
{
    EjsHttpServer   *sp;
    int             i;

    sp = httpGetServerContext(conn->server);
    mprAssert(sp);

    for (i = 0; i < HTTP_TRACE_MAX_DIR; i++) {
        conn->trace[i] = sp->trace[i];
    }
}


static EjsHttpServer *getServerContext(HttpConn *conn)
{
    Ejs             *ejs;
    EjsPath         *dirPath;
    EjsHttpServer   *sp;
    HttpLoc         *loc;
    cchar           *dir;

    if ((sp = httpGetServerContext(conn->server)) != 0) {
        return sp;
    }
    /*
        Hosted handler. Must supply a location block which defines the HttpServer instance.
     */
    loc = conn->rx->loc;
    if (loc == 0 || loc->context == 0) {
        mprError("Location block is not defined for request");
        return 0;
    }
    sp = (EjsHttpServer*) loc->context;
    ejs = sp->ejs;
    dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
    dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->value : conn->host->documentRoot;
    if (sp->server == 0) {
        /* Don't set limits or pipeline. That will come from the embedding server */
        sp->server = conn->server;
        sp->server->ssl = loc->ssl;
        sp->ip = sclone(conn->server->ip);
        sp->port = conn->server->port;
        sp->dir = sclone(dir);
    }
    httpSetServerContext(conn->server, sp);
    httpSetRequestNotifier(conn, (HttpNotifier) stateChangeNotifier);
    return sp;
}


static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn)
{
    Ejs             *ejs;
    EjsRequest      *req;
    EjsPath         *dirPath;
    cchar           *dir;

    ejs = sp->ejs;
    dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
    dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->value : ".";

    req = ejsCreateRequest(ejs, sp, conn, dir);
    httpSetConnContext(conn, req);
#if UNUSED
    conn->dispatcher = ejs->dispatcher;
    conn->tx->handler = ejs->http->ejsHandler;
#endif

#if FUTURE
    if (sp->pipe) {
        def = ejsRunFunction(ejs, sp->createPipeline, 
        if ((vp = ejsGetPropertyByName(ejs, def, ejsName(&name, "", "handler"))) != 0) { 
            handler = ejsToMulti(ejs, vp);
        }
        if ((incoming = ejsGetPropertyByName(ejs, def, ejsName(&name, "", "incoming"))) != 0) { 
            count = ejsGetProperty(ejs, incoming)
            for (i = 0; i < count; i++) {
                mprAddItem(ilist, 
            }
        }
        if ((outgoing = ejsGetPropertyByName(ejs, def, ejsName(&name, "", "outgoing"))) != 0) { 
            count = ejsGetProperty(ejs, incoming)
        }
        if ((connector = ejsGetPropertyByName(ejs, def, ejsName(&name, "", "connector"))) != 0) { 
            connector = ejsToMulti(ejs, vp);
        }
        httpSetPipeline(conn, ejsToMulti(ejs, Handler), ejsToMulti(ejs, connector), 
    }
#endif
    return req;
}


/*
    Note: this may be called multiple times for async, long-running requests.
 */
static void startEjs(HttpQueue *q)
{
    EjsHttpServer   *sp;
    EjsRequest      *req;
    HttpConn        *conn;

    conn = q->conn;
    mprAssert(httpGetConnContext(conn) == 0);
   
    if ((sp = getServerContext(conn)) == 0) {
        return;
    }
    createRequest(sp, conn);
    req = httpGetConnContext(conn);
    if (req && !req->accepted) {
        /* Server accept event */
        req->accepted = 1;
        ejsSendEvent(sp->ejs, sp->emitter, "readable", req, req);
        if (conn->rx->startAfterContent && conn->rx->eof) {
            /* 
                If form or upload, then all content is already buffered and no more input data will arrive. So can send
                the EOF notification here
             */
            HTTP_NOTIFY(conn, 0, HTTP_NOTIFY_READABLE);
        }
    }
}


static void processEjs(HttpQueue *q)
{
    HttpConn        *conn;
    
    conn = q->conn;
#if UNUSED
    EjsHttpServer   *sp;
    EjsRequest      *req;
    sp = httpGetServerContext(conn->server);
    req = httpGetConnContext(conn);
#endif

    if (conn->readq->count > 0) {
        HTTP_NOTIFY(conn, 0, HTTP_NOTIFY_READABLE);
    }
}


/* 
    Create the http pipeline handler for ejs.
 */
HttpStage *ejsAddWebHandler(Http *http, MprModule *module)
{
    HttpStage   *handler;

    mprAssert(http);

    if (http->ejsHandler) {
        return http->ejsHandler;
    }
    handler = httpCreateHandler(http, "ejsHandler", HTTP_STAGE_ALL | HTTP_STAGE_QUERY_VARS | HTTP_STAGE_FORM_VARS, module);
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
static void manageHttpServer(EjsHttpServer *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
#if UNUSED
        mprLog(0, "MARK httpServer for %s", sp->ejs->name);
#endif
        ejsManagePot(sp, flags);
        mprMark(sp->ejs);
        mprMark(sp->server);
        mprMark(sp->sessionTimer);
        mprMark(sp->ssl);
        mprMark(sp->connector);
        mprMark(sp->dir);
        mprMark(sp->keyFile);
        mprMark(sp->certFile);
        mprMark(sp->protocols);
        mprMark(sp->ciphers);
        mprMark(sp->ip);
        mprMark(sp->name);
        mprMark(sp->emitter);
        mprMark(sp->limits);
        mprMark(sp->sessions);
        mprMark(sp->outgoingStages);
        mprMark(sp->incomingStages);
        
    } else {
#if UNUSED
        mprLog(0, "FREE httpServer for %s", sp->ejs->name);
        //  MOB -- can't do this. ejs and everything else could be dead.
        if (!mprIsStopping() && sp->ejs && sp->ejs->service) {
            ejsSendEvent(sp->ejs, sp->emitter, "close", NULL, sp);
        }
#endif
#if UNUSED 
        mprLog(0, "DESTROY HttpServer %s in %s", sp->name, sp->ejs->name);
#endif
        sp->sessions = 0;
        ejsStopSessionTimer(sp);
        if (sp->server) {
            httpDestroyServer(sp->server);
            sp->server = 0;
        }
    }
}


static EjsHttpServer *createHttpServer(Ejs *ejs, EjsType *type, int size)
{
    EjsHttpServer   *sp;

    if ((sp = ejsCreatePot(ejs, type, 0)) == NULL) {
        return NULL;
    }
    sp->ejs = ejs;
#if UNUSED
    static int serverCount = 0;
    //  Messes up the real server name
    sp->name = mprAsprintf("server-%d", serverCount++);
#endif
    sp->async = 1;
    httpInitTrace(sp->trace);
#if UNUSED
    mprLog(0, "CREATE HttpServer %s in %s", sp->name, sp->ejs->name);
#endif
    return sp;
}


void ejsConfigureHttpServerType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs.web", "HttpServer"), sizeof(EjsHttpServer), manageHttpServer, EJS_POT_HELPERS);
    ejs->httpServerType = type;
    type->helpers.create = (EjsCreateHelper) createHttpServer;

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_accept, (EjsProc) hs_accept);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, (EjsProc) hs_address);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, (EjsProc) hs_async, (EjsProc) hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, (EjsProc) hs_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_limits, (EjsProc) hs_limits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_isSecure, (EjsProc) hs_isSecure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, (EjsProc) hs_listen);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_name, (EjsProc) hs_name, (EjsProc) hs_set_name);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, (EjsProc) hs_port);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_off, (EjsProc) hs_off);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_on, (EjsProc) hs_on);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_secure, (EjsProc) hs_secure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setLimits, (EjsProc) hs_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setPipeline, (EjsProc) hs_setPipeline);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_trace, (EjsProc) hs_trace);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_verifyClients, (EjsProc) hs_verifyClients);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_software, (EjsProc) hs_software);

    /* One time initializations */
    ejsLoadHttpService(ejs);
    ejsAddWebHandler(ejs->http, NULL);
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
