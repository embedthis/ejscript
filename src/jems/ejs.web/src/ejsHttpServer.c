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
static EjsHttpServer *lookupServer(Ejs *ejs, cchar *ip, int port);
static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp);
static void setupConnTrace(HttpConn *conn);
static void stateChangeNotifier(HttpConn *conn, int state, int notifyFlags);

/************************************ Code ************************************/
/*  
    function get address(): String
 */
static EjsString *hs_address(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->ip) {
        return ejsCreateStringFromAsc(ejs, sp->ip);
    } 
    return S(null);
}


/*  
    function accept(): Request
 */
static EjsRequest *hs_accept(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
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
    return createRequest(sp, conn);
}


/*  
    function get async(): Boolean
 */
static EjsObj *hs_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (sp->async) ? S(true): S(false);
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
        sp->server = 0;
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
    function setLimits(limits: Object): Void
 */
static EjsObj *hs_setLimits(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    EjsObj      *vp, *app, *cache, *cacheLimits;
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
    }
    if ((vp = ejsGetPropertyByName(ejs, sp->limits, EN("sessionTimeout"))) != 0) {
        app = ejsGetPropertyByName(ejs, ejs->global, N("ejs", "App"));
        cache = ejsGetProperty(ejs, app, ES_App_cache);
        if (cache) {
            cacheLimits = ejsCreateEmptyPot(ejs);
            ejsSetPropertyByName(ejs, cacheLimits, EN("lifespan"), vp);
            ejsCacheSetLimits(ejs, cache, cacheLimits);
        }
    }
    return 0;
}


/*  
    function get isSecure(): Void
 */
static EjsObj *hs_isSecure(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return sp->ssl ? S(true): S(false);
}


/*  
    function listen(endpoint): Void
    An endpoint can be either a "port" or "ip:port", or null
 */
static EjsVoid *hs_listen(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpServer  *server;
    HttpHost    *host;
    EjsString   *address;
    EjsObj      *endpoint, *options;
    EjsPath     *home, *documents;

    endpoint = (argc >= 1) ? argv[0] : S(null);
    if (endpoint != S(null)) {
        address = ejsToString(ejs, endpoint);
        mprParseIp(address->value, &sp->ip, &sp->port, 0);
    } else {
        address = 0;
    }
    if (ejs->hosted) {
        if ((options = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_options)) != 0) {
            sp->hosted = ejsGetPropertyByName(ejs, options, EN("own")) != S(true);
        }
    }

    if (!sp->hosted) {
        if (address == 0) {
            ejsThrowArgError(ejs, "Missing listen endpoint");
            return 0;
        }
        if (sp->server) {
            httpDestroyServer(sp->server);
            sp->server = 0;
        }
        /*
            The server uses the ejsDispatcher. This is VERY important. All connections will inherit this also.
            This serializes all activity on one dispatcher.
         */
        if ((server = httpCreateServer(sp->ip, sp->port, ejs->dispatcher, HTTP_CREATE_HOST)) == 0) {
            ejsThrowIOError(ejs, "Can't create Http server object");
            return 0;
        }
        sp->server = server;
        host = mprGetFirstItem(server->hosts);
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
            httpSetHostName(host, sp->name);
        }
        httpSetSoftware(server->http, EJS_HTTPSERVER_NAME);
        httpSetServerAsync(server, sp->async);
        httpSetServerContext(server, sp);
        httpSetServerNotifier(server, stateChangeNotifier);

        /*
            This is only required for when http is using non-ejs handlers and/or filters
         */
        documents = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_documents);
        if (ejsIs(ejs, documents, Path)) {
            httpSetHostDocumentRoot(host, documents->value);
        }
        home = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_home);
        if (ejsIs(ejs, home, Path)) {
            httpSetHostServerRoot(host, home->value);
        }
        if (httpStartServer(server) < 0) {
            httpDestroyServer(sp->server);
            sp->server = 0;
            ejsThrowIOError(ejs, "Can't listen on %s", address->value);
        }
    }
    if (ejs->httpServers == 0) {
       ejs->httpServers = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    }
    mprRemoveItem(ejs->httpServers, sp);
    mprAddItem(ejs->httpServers, sp);
    return 0;
}


/*  
    function get name(): String
 */
static EjsString *hs_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->name) {
        return ejsCreateStringFromAsc(ejs, sp->name);
    }
    return S(null);
}


/*  
    function set name(hostname: String): Void
 */
static EjsObj *hs_set_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpHost    *host;

    sp->name = ejsToMulti(ejs, argv[0]);
    if (sp->server && sp->name) {
        host = mprGetFirstItem(sp->server->hosts);
        httpSetHostName(host, sp->name);
    }
    return 0;
}


/*  
    function off(name: [String|Array], observer: Function): Void
 */
static EjsObj *hs_off(Ejs *ejs, EjsHttpServer *sp, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, sp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function on(name: [String|Array], observer: Function): Void
 */
static EjsObj *hs_on(Ejs *ejs, EjsHttpServer *sp, int argc, EjsAny **argv)
{
    //  TODO -- should fire if currently readable / writable (also socket etc)
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function get port(): Number
 */
static EjsNumber *hs_port(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, sp->port);
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
    if (!ejsIs(ejs, argv[0], Null)) {
        mprSetSslKeyFile(sp->ssl, ejsToMulti(ejs, argv[0]));
    }
    if (!ejsIs(ejs, argv[1], Null)) {
        mprSetSslCertFile(sp->ssl, ejsToMulti(ejs, argv[1]));
    }
    if (argc >= 3 && ejsIs(ejs, argv[2], Array)) {
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
    if (argc >= 4 && ejsIs(ejs, argv[3], Array)) {
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
static EjsString *hs_software(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, EJS_HTTPSERVER_NAME);
}


/*  
    function verifyClients(caCertPath: Path, caCertFile: Path): Void
 */
static EjsObj *hs_verifyClients(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    //  TODO
    return 0;
}


static void receiveRequest(EjsRequest *req, MprEvent *event)
{
    Ejs             *ejs;
    EjsAny          *argv[1];
    EjsFunction     *onrequest;
    HttpConn        *conn;
    
    conn = req->conn;
    ejs = req->ejs;
    mprAssert(ejs);

    onrequest = ejsGetProperty(ejs, req->server, ES_ejs_web_HttpServer_onrequest);
    if (!ejsIsFunction(ejs, onrequest)) {
        ejsThrowStateError(ejs, "HttpServer.onrequest is not a function");
        return;
    }
    argv[0] = req;
    ejsRunFunction(ejs, onrequest, req->server, 1, argv);
    httpEnableConnEvents(conn);
}


/*
    function passRequest(req: Request, worker: Worker): Void
 */
static EjsVoid *hs_passRequest(Ejs *ejs, EjsHttpServer *server, int argc, EjsAny **argv)
{
    Ejs             *nejs;
    EjsRequest      *req, *nreq;
    EjsWorker       *worker;
    HttpConn        *conn;

    req = argv[0];
    worker = argv[1];

    nejs = worker->pair->ejs;
    conn = req->conn;
    conn->ejs = nejs;
    conn->oldDispatcher = conn->dispatcher;
    conn->newDispatcher = nejs->dispatcher;

    if ((nreq = ejsCloneRequest(nejs, req, 1)) == 0) {
        ejsThrowStateError(ejs, "Can't clone request");
        return 0;
    }
    httpSetConnContext(conn, nreq);

    if ((nreq->server = ejsCloneHttpServer(nejs, req->server, 1)) == 0) {
        ejsThrowStateError(ejs, "Can't clone request");
        return 0;
    }
    conn->workerEvent = mprCreateEvent(conn->dispatcher, "RequestWorker", 0, receiveRequest, nreq, MPR_EVENT_DONT_QUEUE);
    if (conn->workerEvent == 0) {
        ejsThrowStateError(ejs, "Can't create worker event");
    }  
    return 0;
}


/************************************ Support *************************************/

//  TODO rethink this. This should really go into the HttpHost object

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
            if (vs && ejsIs(ejs, vs, String)) {
                name = vs->value;
                if (httpLookupStage(http, name) == 0) {
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
            if (vs && ejsIs(ejs, vs, String)) {
                name = vs->value;
                if (httpLookupStage(http, name) == 0) {
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
            if (req->cloned) {
                ejsSendRequestCloseEvent(req->cloned->ejs, req->cloned);
            }
        }
        break;

    case HTTP_EVENT_IO:
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

    case HTTP_EVENT_CLOSE:
        /* Connection close */
        if (req && req->conn) {
            req->conn = 0;
        }
        break;
    }
}


static void closeEjsHandler(HttpQueue *q)
{
    EjsRequest  *req;
    HttpConn    *conn;

    conn = q->conn;

    if ((req = httpGetConnContext(conn)) != 0) {
        ejsSendRequestCloseEvent(req->ejs, req);
        req->conn = 0;
    }
    httpSetConnContext(conn, 0);
    if (conn->pool && conn->ejs) {
        ejsFreePoolVM(conn->pool, conn->ejs);
        conn->ejs = 0;
    }
}


static void incomingEjsData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpRx          *rx;

    conn = q->conn;
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

    if ((sp = httpGetServerContext(conn->server)) != 0) {
        for (i = 0; i < HTTP_TRACE_MAX_DIR; i++) {
            conn->trace[i] = sp->trace[i];
        }
    }
}


static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn)
{
    Ejs             *ejs;
    EjsRequest      *req;
    cchar           *dir;

    ejs = sp->ejs;
    dir = conn->host->documentRoot;
    req = ejsCreateRequest(ejs, sp, conn, dir);
    httpSetConnContext(conn, req);

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


static void startEjsHandler(HttpQueue *q)
{
    EjsHttpServer   *sp;
    EjsRequest      *req;
    Ejs             *ejs;
    HttpServer      *server;
    HttpConn        *conn;
    MprSocket       *lp;

    conn = q->conn;
    server = conn->server;

    if (conn->ejs == 0) {
        return;
    }
    if ((sp = httpGetServerContext(server)) == 0) {
        mprAssert(conn->ejs);
        lp = conn->sock->listenSock;
        if ((sp = lookupServer(conn->ejs, lp->ip, lp->port)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                    "No HttpServer configured to serve for request from %s:%d", lp->ip, lp->port);
            return;
        }
        ejs = sp->ejs;
        sp->server = server;
        sp->ip = server->ip;
        sp->port = server->port;
        if (!ejsIsDefined(ejs, ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_documents))) {
            ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_documents, 
                ejsCreateStringFromAsc(ejs, conn->host->documentRoot));
        }
    } else {
        ejs = sp->ejs;
    }
    if (conn->notifier == 0) {
        httpSetConnNotifier(conn, stateChangeNotifier);
    }
    if ((req = createRequest(sp, conn)) != 0) {
        ejsSendEvent(ejs, sp->emitter, "readable", req, req);

        /* Send EOF if form or upload and all content has been received.  */
        if (conn->rx->startAfterContent && conn->rx->eof) {
            HTTP_NOTIFY(conn, 0, HTTP_NOTIFY_READABLE);
        }
    }
}


static void processEjsHandler(HttpQueue *q)
{
    HttpConn        *conn;
    
    conn = q->conn;

    if (conn->readq->count > 0) {
        HTTP_NOTIFY(conn, 0, HTTP_NOTIFY_READABLE);
    }
}


/* 
    Create the http pipeline handler
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
    handler->close = closeEjsHandler;
    handler->incomingData = incomingEjsData;
    handler->start = startEjsHandler;
    handler->process = processEjsHandler;
    return handler;
}


/*  
    Mark the object properties for the garbage collector
 */
static void manageHttpServer(EjsHttpServer *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(sp, flags);
        mprMark(sp->ejs);
        mprMark(sp->server);
        mprMark(sp->ssl);
        mprMark(sp->connector);
        mprMark(sp->keyFile);
        mprMark(sp->certFile);
        mprMark(sp->protocols);
        mprMark(sp->ciphers);
        mprMark(sp->ip);
        mprMark(sp->name);
        mprMark(sp->emitter);
        mprMark(sp->limits);
        mprMark(sp->outgoingStages);
        mprMark(sp->incomingStages);
        httpManageTrace(&sp->trace[0], flags);
        httpManageTrace(&sp->trace[1], flags);
        
    } else {
        if (sp->ejs && sp->ejs->httpServers) {
            mprRemoveItem(sp->ejs->httpServers, sp);
        }
        if (!sp->cloned) {
            if (sp->server && !sp->hosted) {
                httpDestroyServer(sp->server);
                sp->server = 0;
            }
        }
    }
}


static EjsHttpServer *createHttpServer(Ejs *ejs, EjsType *type, int size)
{
    EjsHttpServer   *sp;

    if ((sp = ejsCreatePot(ejs, type, 0)) == 0) {
        return 0;
    }
    sp->ejs = ejs;
    sp->async = 1;
    httpInitTrace(sp->trace);
    return sp;
}


EjsHttpServer *ejsCloneHttpServer(Ejs *ejs, EjsHttpServer *sp, bool deep)
{
    EjsHttpServer   *nsp;

    if ((nsp = ejsClonePot(ejs, sp, deep)) == 0) {
        return 0;
    }
    nsp->cloned = sp;
    nsp->ejs = ejs;
    nsp->async = sp->async;
    nsp->server = sp->server;
    nsp->name = sp->name;
    nsp->ssl = sp->ssl;
    nsp->connector = sp->connector;
    nsp->port = sp->port;
    nsp->ip = sp->ip;
    nsp->certFile = sp->certFile;
    nsp->keyFile = sp->keyFile;
    nsp->ciphers = sp->ciphers;
    nsp->protocols = sp->protocols;
    httpInitTrace(nsp->trace);
    return nsp;
}


static EjsHttpServer *lookupServer(Ejs *ejs, cchar *ip, int port)
{
    EjsHttpServer   *sp;
    int             next;

    if (ip == 0) {
        ip = "";
    }
    if (ejs->httpServers) {
        for (next = 0; (sp = mprGetNextItem(ejs->httpServers, &next)) != 0; ) {
            if (sp->port <= 0 || port <= 0 || sp->port == port) {
                if (sp->ip == 0 || *sp->ip == '\0' || *ip == '\0' || scmp(sp->ip, ip) == 0) {
                    return sp;
                }
            }
        }
    }
    return 0;
}


void ejsConfigureHttpServerType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs.web", "HttpServer"), sizeof(EjsHttpServer), manageHttpServer, 
            EJS_TYPE_POT)) == 0) {
        return;
    }
    type->helpers.create = (EjsCreateHelper) createHttpServer;
    type->helpers.clone = (EjsCloneHelper) ejsCloneHttpServer;

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_accept, hs_accept);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, hs_address);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, hs_async, hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, hs_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_limits, hs_limits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_isSecure, hs_isSecure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, hs_listen);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_name, hs_name, hs_set_name);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, hs_port);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_off, hs_off);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_passRequest, hs_passRequest);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_on, hs_on);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_secure, hs_secure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setLimits, hs_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setPipeline, hs_setPipeline);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_trace, hs_trace);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_verifyClients, hs_verifyClients);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_software, hs_software);

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
