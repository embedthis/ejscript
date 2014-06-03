/*
    ejsHttpServer.c -- Ejscript Http Server.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "me.h"

#if ME_EJS_WEB
#include    "ejs.h"
#include    "ejsCompiler.h"
#include    "ejsWeb.h"
#include    "ejs.web.slots.h"

/********************************** Forwards **********************************/

static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn);
static EjsHttpServer *lookupServer(Ejs *ejs, cchar *ip, int port);
static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp);
static void setupConnTrace(HttpConn *conn);
static void stateChangeNotifier(HttpConn *conn, int event, int arg);

/************************************ Code ************************************/
/*  
    function get address(): String
 */
static EjsString *hs_address(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->ip) {
        return ejsCreateStringFromAsc(ejs, sp->ip);
    } 
    return ESV(null);
}


/*  
    function accept(): Request
 */
static EjsRequest *hs_accept(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    MprSocket   *sock;
    HttpConn    *conn;
    MprEvent    event;

    if ((sock = mprAcceptSocket(sp->endpoint->sock)) == 0) {
        /* Just ignore */
        return 0;
    }
    memset(&event, 0, sizeof(MprEvent));
    event.dispatcher = sp->endpoint->dispatcher;
    event.sock = sock;
    if ((conn = httpAcceptConn(sp->endpoint, &event)) == 0) {
        /* Just ignore */
        mprLog("ejs web", 0, "Cannot accept connection");
        return 0;
    }
    return createRequest(sp, conn);
}


/*  
    function get async(): Boolean
 */
static EjsObj *hs_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return (sp->async) ? ESV(true): ESV(false);
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *hs_set_async(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->async = ejsGetBoolean(ejs, argv[0]);
    if (sp->endpoint) {
        httpSetEndpointAsync(sp->endpoint, sp->async);
    }
    return 0;
}


/*
    function get hostedDocuments(): Path
 */
static EjsPath *hs_hostedDocuments(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, ejs->hostedDocuments);
}


/*
    function get hostedHome(): Path
 */
static EjsPath *hs_hostedHome(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return ejsCreatePathFromAsc(ejs, ejs->hostedHome);
}


/*  
    function close(): Void
 */
static EjsObj *hs_close(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (sp->endpoint) {
        ejsSendEvent(ejs, sp->emitter, "close", NULL, sp);
        httpDestroyEndpoint(sp->endpoint);
        sp->endpoint = 0;
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
        limits = (sp->endpoint) ? sp->endpoint->limits : ejs->http->serverLimits;
        assert(limits);
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
        limits = (sp->endpoint) ? sp->endpoint->limits : ejs->http->serverLimits;
        assert(limits);
        ejsGetHttpLimits(ejs, sp->limits, limits, 1);
    }
    ejsBlendObject(ejs, sp->limits, argv[0], EJS_BLEND_OVERWRITE);
    if (sp->endpoint) {
        limits = (sp->endpoint) ? sp->endpoint->limits : ejs->http->serverLimits;
        ejsSetHttpLimits(ejs, limits, sp->limits, 1);
    }
    if ((vp = ejsGetPropertyByName(ejs, sp->limits, EN("sessionTimeout"))) != 0) {
        app = ejsGetPropertyByName(ejs, ejs->global, N("ejs", "App"));
        cache = ejsGetProperty(ejs, app, ES_App_cache);
        if (cache && cache != ESV(null)) {
            cacheLimits = ejsCreateEmptyPot(ejs);
            ejsSetPropertyByName(ejs, cacheLimits, EN("lifespan"), vp);
            ejsCacheSetLimits(ejs, cache, cacheLimits);
        }
    }
    return 0;
}


/*
    function get isSecure(): Boolean
 */
static EjsObj *hs_isSecure(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return sp->ssl ? ESV(true): ESV(false);
}


/*
    function get hosted(): Boolean
 */
static EjsObj *hs_hosted(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return sp->hosted ? ESV(true): ESV(false);
}


/*
    function set hosted(value: Boolean): Void
 */
static EjsVoid *hs_set_hosted(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->hosted = (argv[0] == ESV(true)) ? 1 : 0;
    return 0;
}


/*  
    function listen(endpoint): Void

    An endpoint can be either a "port" or "ip:port", or null. If hosted, this call does little -- just add to the
    ejs->httpServers list.
 */
static EjsVoid *hs_listen(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpEndpoint    *endpoint;
    HttpHost        *host;
    HttpRoute       *route;
    EjsString       *address;
    EjsObj          *loc;
    EjsPath         *documents;

    if (!sp->hosted) {
        loc = (argc >= 1) ? argv[0] : ESV(null);
        if (loc != ESV(null)) {
            address = ejsToString(ejs, loc);
            //  TODO should permit https://IP:PORT
            mprParseSocketAddress(address->value, &sp->ip, &sp->port, NULL, 0);
        } else {
            address = 0;
        }
        if (address == 0) {
            ejsThrowArgError(ejs, "Missing listen endpoint");
            return 0;
        }
        if (sp->endpoint) {
            httpDestroyEndpoint(sp->endpoint);
            sp->endpoint = 0;
        }
        /*
            The endpoint uses the ejsDispatcher. This is VERY important. All connections will inherit this also.
            This serializes all activity on one dispatcher.
         */
        if ((endpoint = httpCreateEndpoint(sp->ip, sp->port, ejs->dispatcher)) == 0) {
            ejsThrowIOError(ejs, "Cannot create Http endpoint object");
            return 0;
        }
        sp->endpoint = endpoint;
        host = httpCreateHost(NULL);
        httpSetHostName(host, sfmt("%s:%d", sp->ip, sp->port));
        route = httpCreateConfiguredRoute(host, 1);
        httpAddRouteMethods(route, "DELETE, HEAD, OPTIONS, PUT");
        httpSetRouteName(route, "default");
        httpAddRouteHandler(route, "ejsHandler", "");
        httpSetRouteTarget(route, "run", 0);
        httpFinalizeRoute(route);
        httpSetHostDefaultRoute(host, route);
        httpAddHostToEndpoint(endpoint, host);

        if (sp->limits) {
            ejsSetHttpLimits(ejs, endpoint->limits, sp->limits, 1);
        }
        if (sp->incomingStages || sp->outgoingStages || sp->connector) {
            setHttpPipeline(ejs, sp);
        }
        if (sp->ssl) {
            httpSecureEndpoint(endpoint, sp->ssl);
        }
        if (sp->name) {
            httpSetHostName(host, sp->name);
        }
        httpSetSoftware(endpoint->http, EJS_HTTPSERVER_NAME);
        httpSetEndpointAsync(endpoint, sp->async);
        httpSetEndpointContext(endpoint, sp);
        httpSetEndpointNotifier(endpoint, stateChangeNotifier);

        /*
            This is only required when http is using non-ejs handlers and/or filters
         */
        documents = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_documents);
        if (ejsIs(ejs, documents, Path)) {
            httpSetRouteDocuments(route, documents->value);
        }
#if KEEP
        //  TODO -- what to do with home?
        //  TODO - if removed, then the "home" property should be removed?
        home = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_home);
        if (ejsIs(ejs, home, Path)) {
            httpSetRoutDir(host, home->value);
        }
#endif
        if (httpStartEndpoint(endpoint) < 0) {
            httpDestroyEndpoint(sp->endpoint);
            sp->endpoint = 0;
            ejsThrowIOError(ejs, "Cannot listen on %s", address->value);
        }
    }
    if (ejs->httpServers == 0) {
       ejs->httpServers = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    }
    /* Remove to make sure old listening() registrations are removed */
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
    return ESV(null);
}


/*  
    function set name(hostname: String): Void
 */
static EjsObj *hs_set_name(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    HttpHost    *host;

    sp->name = ejsToMulti(ejs, argv[0]);
    if (sp->endpoint && sp->name) {
        host = mprGetFirstItem(sp->endpoint->hosts);
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
    function on(name: [String|Array], observer: Function): HttpServer
 */
static EjsHttpServer *hs_on(Ejs *ejs, EjsHttpServer *sp, int argc, EjsAny **argv)
{
    //  TODO -- should fire if currently readable / writable (also socket etc)
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
    return sp;
}


/*  
    function get port(): Number
 */
static EjsNumber *hs_port(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, sp->port);
}


/*  
    function run(): Void
 */
static EjsVoid *hs_run(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    if (!sp->hosted) {
        while (!ejs->exiting && !mprIsStopping()) {
            mprWaitForEvent(ejs->dispatcher, MAXINT); 
        }
    }
    return 0;
}


/*  
    function secure(keyFile: Path, certFile: Path!, protocols: Array? = null, ciphers: Array? = null): Void
 */
static EjsObj *hs_secure(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
#if ME_COM_SSL
    EjsArray    *protocols;
    cchar       *token;
    int         mask, protoMask, i;

    if (sp->ssl == 0 && ((sp->ssl = mprCreateSsl(1)) == 0)) {
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
            if (scaselesscmp(token, "SSLv2") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV2 & ~mask);
                protoMask |= (MPR_PROTO_SSLV2 & mask);

            } else if (scaselesscmp(token, "SSLv3") == 0) {
                protoMask &= ~(MPR_PROTO_SSLV3 & ~mask);
                protoMask |= (MPR_PROTO_SSLV3 & mask);

            } else if (scaselesscmp(token, "TLSv1") == 0) {
                protoMask &= ~(MPR_PROTO_TLSV1 & ~mask);
                protoMask |= (MPR_PROTO_TLSV1 & mask);

            } else if (scaselesscmp(token, "ALL") == 0) {
                protoMask &= ~(MPR_PROTO_ALL & ~mask);
                protoMask |= (MPR_PROTO_ALL & mask);
            }
        }
        mprSetSslProtocols(sp->ssl, protoMask);
    }
    if (argc >= 4 && ejsIs(ejs, argv[3], Array)) {
        mprSetSslCiphers(sp->ssl, ejsToMulti(ejs, argv[3]));
    }
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

    if (sp->endpoint) {
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
    ejsSetupHttpTrace(ejs, sp->trace, argv[0]);
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
    assert(ejs);

    onrequest = ejsGetProperty(ejs, req->server, ES_ejs_web_HttpServer_onrequest);
    if (!ejsIsFunction(ejs, onrequest)) {
        ejsThrowStateError(ejs, "HttpServer.onrequest is not a function");
        return;
    }
    argv[0] = req;
    ejsRunFunction(ejs, onrequest, req->server, 1, argv);
    if (conn->state == HTTP_STATE_BEGIN) {
        conn->ejs = 0;
        httpUsePrimary(conn);        
    }
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
    MprEvent        *event;

    req = argv[0];
    worker = argv[1];

    nejs = worker->pair->ejs;
    conn = req->conn;
    conn->ejs = nejs;

    if ((nreq = ejsCloneRequest(nejs, req, 1)) == 0) {
        ejsThrowStateError(ejs, "Cannot clone request");
        return 0;
    }
    httpSetConnContext(conn, nreq);

    if ((nreq->server = ejsCloneHttpServer(nejs, req->server, 1)) == 0) {
        ejsThrowStateError(ejs, "Cannot clone request");
        return 0;
    }
    event = mprCreateEvent(conn->dispatcher, "RequestWorker", 0, receiveRequest, nreq, MPR_EVENT_DONT_QUEUE);
    httpUseWorker(conn, nejs->dispatcher, event);
    return 0;
}


/************************************ Support *************************************/

//  TODO rethink this. This should really go into the HttpHost object

static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp) 
{
    EjsString       *vs;
    HttpHost        *host;
    HttpRoute       *route;
    Http            *http;
    HttpStage       *stage;
    cchar           *name;
    int             i;

    assert(sp->endpoint);
    http = sp->endpoint->http;
    host = mprGetFirstItem(sp->endpoint->hosts);
    route = mprGetFirstItem(host->routes);

    if (sp->outgoingStages) {
        httpClearRouteStages(route, HTTP_STAGE_TX);
        for (i = 0; i < sp->outgoingStages->length; i++) {
            vs = ejsGetProperty(ejs, sp->outgoingStages, i);
            if (vs && ejsIs(ejs, vs, String)) {
                name = vs->value;
                if (httpLookupStage(http, name) == 0) {
                    ejsThrowArgError(ejs, "Cannot find pipeline stage name %s", name);
                    return;
                }
                httpAddRouteFilter(route, name, NULL, HTTP_STAGE_TX);
            }
        }
    }
    if (sp->incomingStages) {
        httpClearRouteStages(route, HTTP_STAGE_RX);
        for (i = 0; i < sp->incomingStages->length; i++) {
            vs = ejsGetProperty(ejs, sp->incomingStages, i);
            if (vs && ejsIs(ejs, vs, String)) {
                name = vs->value;
                if (httpLookupStage(http, name) == 0) {
                    ejsThrowArgError(ejs, "Cannot find pipeline stage name %s", name);
                    return;
                }
                httpAddRouteFilter(route, name, NULL, HTTP_STAGE_RX);
            }
        }
    }
    if (sp->connector) {
        if ((stage = httpLookupStage(http, sp->connector)) == 0) {
            ejsThrowArgError(ejs, "Cannot find pipeline stage name %s", sp->connector);
            return;
        }
        route->connector = stage;
    }
}


/*
    Notification callback. This routine is called from the Http pipeline on connection state changes. 
 */
static void stateChangeNotifier(HttpConn *conn, int event, int arg)
{
    Ejs             *ejs;
    EjsRequest      *req;

    assert(conn);

    ejs = 0;
    if ((req = httpGetConnContext(conn)) != 0) {
        ejs = req->ejs;
    }
    switch (event) {
    case HTTP_EVENT_STATE:
        if (arg == HTTP_STATE_BEGIN) {
            setupConnTrace(conn);
        } else if (arg == HTTP_STATE_FINALIZED) {
            if (req) {
                if (conn->error) {
                    ejsSendRequestErrorEvent(ejs, req);
                }
                ejsSendRequestCloseEvent(ejs, req);
                if (req->cloned) {
                    ejsSendRequestCloseEvent(req->ejs, req->cloned);
                }
            }
        }
        break;

    case HTTP_EVENT_READABLE:
        /*  IO event notification for the request.  */
        if (req && req->emitter) {
            ejsSendEvent(ejs, req->emitter, "readable", NULL, req);
        } 
        break;

    case HTTP_EVENT_WRITABLE:
        if (req && req->emitter) {
            ejsSendEvent(ejs, req->emitter, "writable", NULL, req);
        }
        break;

    case HTTP_EVENT_APP_CLOSE:
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


static void incomingEjs(HttpQueue *q, HttpPacket *packet)
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
    } else {
        httpJoinPacketForService(q, packet, 0);
    }
    HTTP_NOTIFY(q->conn, HTTP_EVENT_READABLE, 0);
}


static void setupConnTrace(HttpConn *conn)
{
    EjsHttpServer   *sp;

    assert(conn->endpoint);
    if (conn->endpoint) {
        if ((sp = httpGetEndpointContext(conn->endpoint)) != 0) {
            conn->trace = sp->trace;
        }
    }
}


static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn)
{
    Ejs             *ejs;
    EjsRequest      *req;
    EjsPath         *documents;
    cchar           *dir;

    ejs = sp->ejs;
    documents = ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_documents);
    if (ejsIs(ejs, documents, Path)) {
        dir = documents->value;
    } else {
        /* Safety fall back */
        dir = conn->rx->route->home;
    }
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
    HttpEndpoint    *endpoint;
    HttpConn        *conn;
    HttpRx          *rx;
    MprSocket       *lp;

    conn = q->conn;
    rx = conn->rx;
    endpoint = conn->endpoint;

    if (conn->error) {
        return;
    }
    if ((sp = httpGetEndpointContext(endpoint)) == 0) {
        lp = conn->sock->listenSock;
        if ((sp = lookupServer(conn->ejs, lp->ip, lp->port)) == 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                    "No HttpServer configured to serve for request from %s:%d", lp->ip, lp->port);
            return;
        }
        ejs = sp->ejs;
        sp->endpoint = endpoint;
        sp->ip = endpoint->ip;
        sp->port = endpoint->port;
        if (!ejsIsDefined(ejs, ejsGetProperty(ejs, sp, ES_ejs_web_HttpServer_documents))) {
            ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_documents, ejsCreateStringFromAsc(ejs, conn->rx->route->home));
        }
    } else if (conn->ejs) {
        ejs = conn->ejs;
    } else {
        ejs = sp->ejs;
    }
    assert(!conn->tx->finalized);
    if (conn->notifier == 0) {
        httpSetConnNotifier(conn, stateChangeNotifier);
        assert(!conn->tx->finalized);
    }
    if ((req = createRequest(sp, conn)) != 0) {
        assert(!conn->tx->finalized);
        ejsSendEvent(ejs, sp->emitter, "readable", req, req);

        /* Send EOF if form or upload and all content has been received.  */
        if ((rx->form || rx->upload) && rx->eof) {
            HTTP_NOTIFY(conn, HTTP_EVENT_READABLE, 0);
        }
    }
}


static void readyEjsHandler(HttpQueue *q)
{
    HttpConn        *conn;
    
    conn = q->conn;
    if (conn->readq->count > 0) {
        HTTP_NOTIFY(conn, HTTP_EVENT_READABLE, 0);
    }
}


/* 
    Create the http pipeline handler
 */
HttpStage *ejsAddWebHandler(Http *http, MprModule *module)
{
    HttpStage   *handler;

    assert(http);
    if ((handler = http->ejsHandler) == 0) {
        if ((handler = httpCreateHandler(http, "ejsHandler", module)) == 0) {
            return 0;
        }
    }
    http->ejsHandler = handler;
    handler->close = closeEjsHandler;
    handler->incoming = incomingEjs;
    handler->start = startEjsHandler;
    handler->ready = readyEjsHandler;
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
        mprMark(sp->endpoint);
        mprMark(sp->ssl);
        mprMark(sp->trace);
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
        
    } else {
        if (sp->ejs && sp->ejs->httpServers) {
            mprRemoveItem(sp->ejs->httpServers, sp);
        }
        if (!sp->cloned) {
            if (sp->endpoint && !sp->hosted) {
                httpDestroyEndpoint(sp->endpoint);
                sp->endpoint = 0;
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
    sp->hosted = ejs->hosted;
    sp->async = 1;
    sp->trace = httpCreateTrace(0);
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
    nsp->endpoint = sp->endpoint;
    nsp->name = sp->name;
    nsp->ssl = sp->ssl;
    nsp->connector = sp->connector;
    nsp->port = sp->port;
    nsp->ip = sp->ip;
    nsp->certFile = sp->certFile;
    nsp->keyFile = sp->keyFile;
    nsp->ciphers = sp->ciphers;
    nsp->protocols = sp->protocols;
    nsp->trace = httpCreateTrace(sp->trace);
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
            EJS_TYPE_POT | EJS_TYPE_DYNAMIC_INSTANCES)) == 0) {
        return;
    }
    type->helpers.create = (EjsCreateHelper) createHttpServer;
    type->helpers.clone = (EjsCloneHelper) ejsCloneHttpServer;

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_accept, hs_accept);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, hs_address);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, hs_async, hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_hostedDocuments, hs_hostedDocuments);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_hostedHome, hs_hostedHome);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, hs_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_limits, hs_limits);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_hosted, hs_hosted, hs_set_hosted);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_isSecure, hs_isSecure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, hs_listen);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_name, hs_name, hs_set_name);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, hs_port);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_off, hs_off);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_on, hs_on);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_passRequest, hs_passRequest);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_run, hs_run);
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
#endif


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
