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
static void setHttpPipeline(Ejs *ejs, EjsHttpServer *sp);
static void setupConnTrace(HttpConn *conn);
static void stateChangeNotifier(HttpConn *conn, int state, int notifyFlags);

/************************************ Code ************************************/
/*  
    function HttpServer(documentRoot: Path = ".", serverRoot: Path = ".")
    Constructor function
 */
static EjsObj *hs_HttpServer(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    EjsObj      *serverRoot, *documentRoot;

    sp->ejs = ejs;
    sp->async = 1;

    documentRoot = (argc >= 1) ? argv[0] : (EjsObj*) ejsCreatePath(ejs, ".");
    ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_documentRoot, documentRoot);

    serverRoot = (argc >= 2) ? argv[1] : (EjsObj*) ejsCreatePath(ejs, ".");
    ejsSetProperty(ejs, sp, ES_ejs_web_HttpServer_serverRoot, serverRoot);

#if UNUSED
    sp->traceLevel = HTTP_TRACE_LEVEL;
    sp->traceMask = HTTP_TRACE_TRANSMIT | HTTP_TRACE_RECEIVE | HTTP_TRACE_CONN | HTTP_TRACE_FIRST | HTTP_TRACE_HEADERS;
    sp->traceMaxLength = INT_MAX;
#endif
    httpInitTrace(sp->trace);
    return (EjsObj*) sp;
}


/*  
    function observe(name: [String|Array], observer: Function): Void
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
        /* Just ignore */
        mprError(ejs, "Can't accept connection");
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
        //  MOB -- who make sure that the server object is permanent?
        ejsSendEvent(ejs, sp->emitter, "close", NULL, (EjsObj*) sp);
        mprFree(sp->server);
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
        sp->limits = ejsCreateSimpleObject(ejs);
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
    HttpLimits  *limits;
    
    if (sp->limits == 0) {
        sp->limits = ejsCreateSimpleObject(ejs);
        limits = (sp->server) ? sp->server->limits : ejs->http->serverLimits;
        mprAssert(limits);
        ejsGetHttpLimits(ejs, sp->limits, limits, 1);
    }
    ejsBlendObject(ejs, sp->limits, argv[0], 1);
    if (sp->server) {
        ejsSetHttpLimits(ejs, sp->server->limits, sp->limits, 1);
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
        if (ejs->loc) {
            ejs->loc->context = sp;
        } else {
            ejsThrowStateError(ejs, "Can't find web server context for Ejscript. Check EjsStartup directive");
            return 0;
        }
        return (EjsObj*) ejs->nullValue;
    }
    if (ejs->loc) {
        /* Being called hosted - ignore endpoint value */
        sp->obj.permanent = 1;
        ejs->loc->context = sp;
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
    if (sp->limits) {
        ejsSetHttpLimits(ejs, sp->server->limits, sp->limits, 1);
    }
    if (sp->incomingStages || sp->outgoingStages || sp->connector) {
        setHttpPipeline(ejs, sp);
    }
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
    root = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_serverRoot);
    if (ejsIsPath(ejs, root)) {
        server->serverRoot = mprStrdup(server, root->path);
    }

    //  MOB -- who make sure that the sp object is permanent?
    //      or (better) have  a destructor that closes the entire connection (and all requests) if goes out of scope

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
    function removeObserver(name: [String|Array], observer: Function): Void
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
    function setPipeline(incoming: Array, outgoing: Array, connector: String): Void
 */
static EjsObj *hs_setPipeline(Ejs *ejs, EjsHttpServer *sp, int argc, EjsObj **argv)
{
    sp->incomingStages = (EjsArray*) argv[0];
    sp->outgoingStages = (EjsArray*) argv[1];
    sp->connector = ejsGetString(ejs, argv[2]);

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
    ejsSetupTrace(ejs, sp, sp->trace, argv[0]);
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
#if UNUSED
/*
    Get limits:  obj[*] = limits
 */
void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, int server) 
{
    EjsName     qname;

    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "chunk"), ejsCreateNumber(ejs, limits->chunkSize));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "receive"), ejsCreateNumber(ejs, limits->receiveBodySize));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "reuse"), ejsCreateNumber(ejs, limits->keepAliveCount));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "transmission"), ejsCreateNumber(ejs, limits->transmissionBodySize));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "upload"), ejsCreateNumber(ejs, limits->uploadSize));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "inactivityTimeout"), 
        ejsCreateNumber(ejs, limits->inactivityTimeout / MPR_TICKS_PER_SEC));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "requestTimeout"), 
        ejsCreateNumber(ejs, limits->requestTimeout / MPR_TICKS_PER_SEC));
    ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "sessionTimeout"), 
        ejsCreateNumber(ejs, limits->sessionTimeout / MPR_TICKS_PER_SEC));

    if (server) {
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "clients"), ejsCreateNumber(ejs, limits->clientCount));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "header"), ejsCreateNumber(ejs, limits->headerSize));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "headers"), ejsCreateNumber(ejs, limits->headerCount));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "requests"), ejsCreateNumber(ejs, limits->requestCount));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "sessions"), ejsCreateNumber(ejs, limits->sessionCount));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "stageBuffer"), ejsCreateNumber(ejs, limits->stageBufferSize));
        ejsSetPropertyByName(ejs, obj, ejsName(&qname, "", "uri"), ejsCreateNumber(ejs, limits->uriSize));
    }
}


/*
    Set the limit field:    *limit = obj[field]
 */
static void setLimit(Ejs *ejs, EjsObj *obj, cchar *field, int *limit, int factor)
{
    EjsObj      *vp;
    EjsName     qname;

    if ((vp = ejsGetPropertyByName(ejs, obj, ejsName(&qname, "", field))) != 0) {
        *limit = ejsGetInt(ejs, ejsToNumber(ejs, vp)) * factor;
    }
}


void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, int server) 
{
    setLimit(ejs, obj, "chunk", &limits->chunkSize, 1);
    setLimit(ejs, obj, "inactivityTimeout", &limits->inactivityTimeout, MPR_TICKS_PER_SEC);
    setLimit(ejs, obj, "receive", &limits->receiveBodySize, 1);
    setLimit(ejs, obj, "reuse", &limits->keepAliveCount, 1);
    setLimit(ejs, obj, "requestTimeout", &limits->requestTimeout, MPR_TICKS_PER_SEC);
    setLimit(ejs, obj, "sessionTimeout", &limits->sessionTimeout, MPR_TICKS_PER_SEC);
    setLimit(ejs, obj, "transmission", &limits->transmissionBodySize, 1);
    setLimit(ejs, obj, "upload", &limits->uploadSize, 1);

    if (server) {
        setLimit(ejs, obj, "clients", &limits->clientCount, 1);
        setLimit(ejs, obj, "requests", &limits->requestCount, 1);
        setLimit(ejs, obj, "sessions", &limits->sessionCount, 1);
        setLimit(ejs, obj, "stageBuffer", &limits->stageBufferSize, 1);
        setLimit(ejs, obj, "uri", &limits->uriSize, 1);
        setLimit(ejs, obj, "headers", &limits->headerCount, 1);
        setLimit(ejs, obj, "header", &limits->headerSize, 1);
    }
}
#endif


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
            if (vs && ejsIsString(vs)) {
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
            if (vs && ejsIsString(vs)) {
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

    case HTTP_STATE_PARSED:
        conn->tx->handler = (conn->error) ? conn->http->passHandler : conn->http->ejsHandler;
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
                ejsSendEvent(ejs, req->emitter, "readable", NULL, (EjsObj*) req);
            } 
            if (notifyFlags & HTTP_NOTIFY_WRITABLE) {
                ejsSendEvent(ejs, req->emitter, "writable", NULL, (EjsObj*) req);
            }
        }
        break;
    }
}


static void closeEjs(HttpQueue *q)
{
    EjsRequest  *req;

    if ((req = httpGetConnContext(q->conn)) != 0) {
        if (!req->closed) {
            ejsSendRequestCloseEvent(req->ejs, req);
        }
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
        if (rx->form) {
            httpAddVarsFromQueue(q);
        }
        HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);

    } else if (conn->writeComplete) {
        httpFreePacket(q, packet);

    } else {
        httpJoinPacketForService(q, packet, 0);
        HTTP_NOTIFY(q->conn, 0, HTTP_NOTIFY_READABLE);
    }
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


static EjsRequest *createRequest(EjsHttpServer *sp, HttpConn *conn)
{
    Ejs             *ejs;
    EjsRequest      *req;
    EjsPath         *dirPath;
    HttpLoc         *loc;
    cchar           *dir;

    if (conn->tx->handler->match) {
        /*
            Hosted handler. Must supply a location block which defines the HttpServer instance.
         */
        loc = conn->rx->loc;
        if (loc == 0 || loc->context == 0) {
            mprError(sp, "Location block is not defined for request");
            return 0;
        }
        sp = (EjsHttpServer*) loc->context;
        ejs = sp->ejs;
        dirPath = ejsGetProperty(ejs, (EjsObj*) sp, ES_ejs_web_HttpServer_documentRoot);
        dir = (dirPath && ejsIsPath(ejs, dirPath)) ? dirPath->path : conn->documentRoot;
        if (sp->server == 0) {
            /* Don't set limits or pipeline. That will come from the embedding server */
            sp->server = conn->server;
            sp->server->ssl = loc->ssl;
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
    conn->tx->handler = ejs->http->ejsHandler;

#if FUTURE
    if (sp->pipe) {
        def = ejsRunFunction(ejs, sp->createPipeline, 
        if ((vp = ejsGetPropertyByName(ejs, def, ejsName(&name, "", "handler"))) != 0) { 
            handler = ejsGetString(ejs, vp);
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
            connector = ejsGetString(ejs, vp);
        }
        httpSetPipeline(conn, ejsGetString(ejs, Handler), 
            ejsGetString(ejs, connector), 
    }
#endif
    return req;
}


static void runEjs(HttpQueue *q)
{
    EjsHttpServer   *sp;
    EjsRequest      *req;
    HttpConn        *conn;

    conn = q->conn;
    if (!conn->abortPipeline) {
        sp = httpGetServerContext(conn->server);
        mprAssert(sp);
        if ((req = httpGetConnContext(conn)) == 0 && (req = createRequest(sp, conn)) == 0) {
            return;
        }
        if (!req->accepted) {
            /* Server accept event */
            req->accepted = 1;
            ejsSendEvent(sp->ejs, sp->emitter, "readable", (EjsObj*) req, (EjsObj*) req);
        }
    }
}


static void startEjs(HttpQueue *q)
{
    HttpRx      *rx;

    rx = q->conn->rx;
    if (!rx->form && !rx->upload) {
        runEjs(q);
    }
}


static void processEjs(HttpQueue *q)
{
    HttpRx      *rx;

    rx = q->conn->rx;
    if (rx->form || rx->upload) {
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
    ejsMarkObject(ejs, (EjsObj*) sp);
    if (sp->emitter) {
        ejsMark(ejs, (EjsObj*) sp->emitter);
    }
    if (sp->limits) {
        ejsMark(ejs, (EjsObj*) sp->limits);
    }
    if (sp->outgoingStages) {
        ejsMark(ejs, (EjsObj*) sp->outgoingStages);
    }
    if (sp->incomingStages) {
        ejsMark(ejs, (EjsObj*) sp->incomingStages);
    }
}


static void destroyHttpServer(Ejs *ejs, EjsHttpServer *sp)
{
    ejsSendEvent(ejs, sp->emitter, "close", NULL, (EjsObj*) sp);
    mprFree(sp->server);
    sp->server = 0;
}


void ejsConfigureHttpServerType(Ejs *ejs)
{
    EjsObj      *prototype;
    EjsType     *type;

    type = ejsConfigureNativeType(ejs, "ejs.web", "HttpServer", sizeof(EjsHttpServer));
    type->helpers.mark = (EjsMarkHelper) markHttpServer;
    type->helpers.destroy = (EjsDestroyHelper) destroyHttpServer;

    prototype = type->prototype;
    ejsBindConstructor(ejs, type, (EjsProc) hs_HttpServer);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_accept, (EjsProc) hs_accept);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_address, (EjsProc) hs_address);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_async, (EjsProc) hs_async, (EjsProc) hs_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_close, (EjsProc) hs_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_limits, (EjsProc) hs_limits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_isSecure, (EjsProc) hs_isSecure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_listen, (EjsProc) hs_listen);
    ejsBindAccess(ejs, prototype, ES_ejs_web_HttpServer_name, (EjsProc) hs_name, (EjsProc) hs_set_name);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_port, (EjsProc) hs_port);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_observe, (EjsProc) hs_observe);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_removeObserver, (EjsProc) hs_removeObserver);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_secure, (EjsProc) hs_secure);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setLimits, (EjsProc) hs_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_setPipeline, (EjsProc) hs_setPipeline);
    ejsBindMethod(ejs, prototype, ES_ejs_web_HttpServer_trace, (EjsProc) hs_trace);
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
