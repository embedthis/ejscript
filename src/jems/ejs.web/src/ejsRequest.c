/*
    ejsRequest.c -- Ejscript web framework.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsWeb.h"
#include    "ecCompiler.h"
#include    "ejs.web.slots.h"

/********************************** Forwards **********************************/

static void defineParam(Ejs *ejs, EjsObj *params, cchar *key, cchar *value);

/************************************* Code ***********************************/
 
static void defineParam(Ejs *ejs, EjsObj *params, cchar *key, cchar *svalue)
{
    EjsName     qname;
    EjsObj      *vp, *value;
    char        *subkey, *end;
    int         slotNum;

    mprAssert(params);

    if (*svalue == '[') {
        value = ejsDeserialize(ejs, ejsCreateString(ejs, svalue));
    } else {
        value = (EjsVar*) ejsCreateString(ejs, svalue);
    }

    /*  
        name.name.name
     */
    if (strchr(key, '.') == 0) {
        ejsName(&qname, "", key);
        ejsSetPropertyByName(ejs, params, &qname, value);

    } else {
        subkey = mprStrdup(ejs, key);
        for (end = strchr(subkey, '.'); end; subkey = end, end = strchr(subkey, '.')) {
            *end++ = '\0';
            ejsName(&qname, "", subkey);
            vp = ejsGetPropertyByName(ejs, params, &qname);
            if (vp == 0) {
                slotNum = ejsSetPropertyByName(ejs, params, &qname, ejsCreateSimpleObject(ejs));
                vp = ejsGetProperty(ejs, params, slotNum);
            }
            params = vp;
        }
        mprAssert(params);
        ejsName(&qname, "", subkey);
        ejsSetPropertyByName(ejs, params, &qname, value);
    }
}


static EjsObj *createParams(Ejs *ejs, EjsRequest *req)
{
    EjsObj          *params;
    MprHashTable    *formVars;
    MprHash         *hp;

    if ((params = req->params) == 0) {
        params = (EjsObj*) ejsCreateSimpleObject(ejs);
        if ((formVars = req->conn->receiver->formVars) != 0) {
            hp = 0;
            while ((hp = mprGetNextHash(formVars, hp)) != NULL) {
                defineParam(ejs, params, hp->key, hp->data);
            }
        }
    }
    return req->params = params;
}


static EjsObj *createCookies(Ejs *ejs, EjsRequest *req)
{
    EjsName     n;
    EjsObj      *argv[1];
    cchar       *cookieHeader;

    if (req->cookies) {
        return (EjsObj*) req->cookies;
    }
    if ((cookieHeader = mprLookupHash(req->conn->receiver->headers, "cookie")) == 0) {
        req->cookies = (EjsObj*) ejs->nullValue;
    } else {
        argv[0] = (EjsObj*) ejsCreateString(ejs, cookieHeader);
        req->cookies = ejsRunFunctionByName(ejs, ejs->global, ejsName(&n, "ejs.web", "parseCookies"), 
            ejs->global, 1, argv);
    }
    return (EjsObj*) req->cookies;
}


static EjsObj *createEnv(Ejs *ejs, EjsRequest *req)
{
    if (req->env) {
        req->env = ejsCreateSimpleObject(ejs);
    }
    return (EjsObj*) req->env;
}


/*
    This will get the current session or create a new session if required
 */
static EjsObj *getSession(Ejs *ejs, EjsRequest *req, int create)
{
    if (req->session) {
        return (EjsObj*) req->session;
    }
    if ((req->session = ejsGetSession(ejs, req)) == NULL && create) {
        req->session = ejsCreateSession(ejs, req, 0, 0);
    }
    if (req->session) {
        //  TODO - SECURE (last arg) ?
        httpSetCookie(req->conn, EJS_SESSION, req->session->id, "/", NULL, 0, 0);
    }
    return (EjsObj*) req->session;
}


static EjsObj *createString(Ejs *ejs, cchar *value)
{
    if (value == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, value);
}


static EjsObj *createHeaders(Ejs *ejs, EjsRequest *req)
{
    MprHash     *hp;
    HttpConn    *conn;
    EjsName     n;
    
    conn = req->conn;
    if (req->headers == 0) {
        req->headers = (EjsObj*) ejsCreateSimpleObject(ejs);
        for (hp = 0; (hp = mprGetNextHash(conn->receiver->headers, hp)) != 0; ) {
            ejsSetPropertyByName(ejs, req->headers, EN(&n, hp->key), ejsCreateString(ejs, hp->data));
        }
    }
    return (EjsObj*) req->headers;
}


static EjsObj *createFiles(Ejs *ejs, EjsRequest *req)
{
    HttpUploadFile  *up;
    HttpConn        *conn;
    EjsObj          *files, *file;
    EjsName         n;
    MprHash         *hp;
    int             index;

    if (req->files == 0) {
        conn = req->conn;
        if (conn->receiver->files == 0) {
            return ejs->nullValue;
        }
        req->files = files = (EjsObj*) ejsCreateSimpleObject(ejs);
        for (index = 0, hp = 0; (hp = mprGetNextHash(conn->receiver->files, hp)) != 0; index++) {
            up = (HttpUploadFile*) hp->data;
            file = (EjsObj*) ejsCreateSimpleObject(ejs);
            ejsSetPropertyByName(ejs, file, EN(&n, "filename"), ejsCreateString(ejs, up->filename));
            ejsSetPropertyByName(ejs, file, EN(&n, "clientFilename"), ejsCreateString(ejs, up->clientFilename));
            ejsSetPropertyByName(ejs, file, EN(&n, "contentType"), ejsCreateString(ejs, up->contentType));
            ejsSetPropertyByName(ejs, file, EN(&n, "name"), ejsCreateString(ejs, hp->key));
            ejsSetPropertyByName(ejs, file, EN(&n, "size"), ejsCreateNumber(ejs, up->size));
            ejsSetPropertyByName(ejs, files, EN(&n, hp->key), file);
        }
    }
    return (EjsObj*) req->files;
}


static char *makeRelativeHome(Ejs *ejs, EjsRequest *req)
{
    HttpReceiver    *rec;
    cchar           *path, *end, *sp;
    char            *home, *cp;
    int             levels;

    rec = req->conn->receiver;
    mprAssert(rec->pathInfo);

    path = rec->pathInfo;
    end = &path[strlen(path)];
    for (levels = 1, sp = &path[1]; sp < end; sp++) {
        if (*sp == '/' && sp[-1] != '/') {
            levels++;
        }
    }
    home = mprAlloc(req, levels * 3 + 2);
    if (levels) {
        for (cp = home; levels > 0; levels--) {
            strcpy(cp, "../");
            cp += 3;
        }
        *cp = '\0';
    } else {
        strcpy(home, "./");
    }
    return home;
}


/*
    Lookup a property. These properties are virtualized.
 */
static EjsObj *getRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum)
{
    HttpConn        *conn;
    HttpReceiver    *rec;
    EjsName         n;
    EjsObj          *value;

    conn = req->conn;
    rec = conn->receiver;

    switch (slotNum) {
    case ES_ejs_web_Request_absHome:
        return (EjsObj*) ejsCreateUri(ejs, req->absHome);

    case ES_ejs_web_Request_authGroup:
        return createString(ejs, conn->authGroup);

    case ES_ejs_web_Request_authType:
        return createString(ejs, conn->authType);

    case ES_ejs_web_Request_authUser:
        return createString(ejs, conn->authUser);

    case ES_ejs_web_Request_chunkSize:
        return (EjsObj*) ejsCreateBoolean(ejs, httpGetChunkSize(conn));

    case ES_ejs_web_Request_config:
        value = ejs->objectType->helpers.getProperty(ejs, (EjsObj*) req, slotNum);
        if (value == (EjsObj*) ejs->nullValue) {
            /* Default to App.config */
            value = ejsGetProperty(ejs, (EjsObj*) ejs->appType, ES_App_config);
        }
        return value;

    case ES_ejs_web_Request_contentLength:
        return (EjsObj*) ejsCreateNumber(ejs, rec->length);

    case ES_ejs_web_Request_contentType:
        createHeaders(ejs, req);
        return ejsGetPropertyByName(ejs, req->headers, EN(&n, "content-type"));

    case ES_ejs_web_Request_cookies:
        return createCookies(ejs, req);

    case ES_ejs_web_Request_dir:
        return (EjsObj*) ejsCreatePath(ejs, req->dir);

    case ES_ejs_web_Request_env:
        return createEnv(ejs, req);

    case ES_ejs_web_Request_files:
        return createFiles(ejs, req);

    case ES_ejs_web_Request_headers:
        return createHeaders(ejs, req);

    case ES_ejs_web_Request_home:
        return (EjsObj*) ejsCreateUri(ejs, req->home);

    case ES_ejs_web_Request_host:
        return createString(ejs, rec->hostName);

    case ES_ejs_web_Request_method:
        return createString(ejs, rec->method);

    case ES_ejs_web_Request_params:
        return createParams(ejs, req);

    case ES_ejs_web_Request_pathInfo:
        return createString(ejs, rec->pathInfo);

    case ES_ejs_web_Request_query:
        return createString(ejs, rec->parsedUri->query);

    case ES_ejs_web_Request_referrer:
        return createString(ejs, rec->referer);

    case ES_ejs_web_Request_remoteAddress:
        return createString(ejs, conn->ip);

    case ES_ejs_web_Request_scheme:
        return createString(ejs, conn->protocol);

    case ES_ejs_web_Request_scriptName:
        return createString(ejs, rec->scriptName);

    case ES_ejs_web_Request_secure:
        return (EjsObj*) ejsCreateBoolean(ejs, conn->secure);

    case ES_ejs_web_Request_server:
        return (EjsObj*) req->server;

    case ES_ejs_web_Request_session:
        return getSession(ejs, req, 1);

    case ES_ejs_web_Request_sessionID:
        if (!req->probedSession) {
            getSession(ejs, req, 0);
            req->probedSession = 1;
        }
        if (req->session) {
            return createString(ejs, req->session->id);
        } else return ejs->nullValue;

    case ES_ejs_web_Request_status:
        return (EjsObj*) ejsCreateNumber(ejs, conn->transmitter->status);

    case ES_ejs_web_Request_timeout:
        return (EjsObj*) ejsCreateNumber(ejs, conn->timeout);

    case ES_ejs_web_Request_uri:
        return (EjsObj*) ejsCreateUri(ejs, rec->uri);

    case ES_ejs_web_Request_userAgent:
        return createString(ejs, rec->userAgent);

    default:
        if (slotNum < req->obj.numSlots) {
            return ejs->objectType->helpers.getProperty(ejs, (EjsObj*) req, slotNum);
        }
    }
    return 0;
}


static int getRequestPropertyCount(Ejs *ejs, EjsRequest *req)
{
    return ES_ejs_web_Request_NUM_INSTANCE_PROP;
}


static EjsName getRequestPropertyName(Ejs *ejs, EjsRequest *req, int slotNum)
{
    return ejsGetPropertyName(ejs, (EjsObj*) req->obj.type->prototype, slotNum);
}


static int lookupRequestProperty(Ejs *ejs, EjsRequest *req, EjsName *qname)
{
    return ejsLookupProperty(ejs, (EjsObj*) req->obj.type->prototype, qname);
}


static int getNum(Ejs *ejs, EjsObj *vp)
{
    if (!ejsIsNumber(vp) && (vp = (EjsObj*) ejsToNumber(ejs, vp)) == 0) {
        return 0;
    }
    return (int) ((EjsNumber*) vp)->value;
}


static cchar *getString(Ejs *ejs, EjsObj *vp)
{
    if (!ejsIsString(vp) && (vp = (EjsObj*) ejsToString(ejs, vp)) == 0) {
        return 0;
    }
    return ((EjsString*) vp)->value;
}


static int setRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum,  EjsObj *value)
{
    HttpConn        *conn;
    HttpReceiver    *rec;

    conn = req->conn;
    rec = conn->receiver;

    switch (slotNum) {
    case ES_ejs_web_Request_absHome:
        req->absHome = mprStrdup(req, getString(ejs, value));
        break;

    case ES_ejs_web_Request_chunkSize:
        httpSetChunkSize(conn, getNum(ejs, value));
        break;

    case ES_ejs_web_Request_dir:
        req->dir = mprStrdup(req, getString(ejs, value));
        break;

    case ES_ejs_web_Request_home:
        req->home = mprStrdup(req, getString(ejs, value));
        break;

    case ES_ejs_web_Request_pathInfo:
        mprFree(rec->pathInfo);
        rec->pathInfo = mprStrdup(rec, getString(ejs, value));
        break;

    case ES_ejs_web_Request_scriptName:
        mprFree(rec->scriptName);
        rec->scriptName = mprStrdup(rec, getString(ejs, value));
        break;

    case ES_ejs_web_Request_server:
        //  MOB -- should validate type
        req->server = (EjsHttpServer*) value;
        break;

    case ES_ejs_web_Request_status:
        httpSetStatus(conn, getNum(ejs, value));
        break;

    case ES_ejs_web_Request_timeout:
        httpSetTimeout(conn, getNum(ejs, value));
        break;

    case ES_ejs_web_Request_authGroup:
    case ES_ejs_web_Request_authType:
    case ES_ejs_web_Request_authUser:
    case ES_ejs_web_Request_contentLength:
    case ES_ejs_web_Request_contentType:
    case ES_ejs_web_Request_cookies:
    case ES_ejs_web_Request_env:
    case ES_ejs_web_Request_files:
    case ES_ejs_web_Request_headers:
    case ES_ejs_web_Request_host:
    case ES_ejs_web_Request_method:
    case ES_ejs_web_Request_params:
    case ES_ejs_web_Request_query:
    case ES_ejs_web_Request_referrer:
    case ES_ejs_web_Request_remoteAddress:
    case ES_ejs_web_Request_scheme:
    case ES_ejs_web_Request_secure:
    case ES_ejs_web_Request_session:
    case ES_ejs_web_Request_sessionID:
    case ES_ejs_web_Request_uri:
    case ES_ejs_web_Request_userAgent:
        ejsThrowReferenceError(ejs, "Property is readonly");
        break;

    case ES_ejs_web_Request_config:
    default:
        return ejs->objectType->helpers.setProperty(ejs, (EjsObj*) req, slotNum, value);
    }
    return 0;
}


/******************************************************************************/
/*  
    function observe(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_observe(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    //  MOB - must issue writable + readable if data available
    ejsAddListener(ejs, &req->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function get async(): Boolean
 */
static EjsObj *req_async(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    return httpGetAsync(req->conn) ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *req_set_async(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    httpSetAsync(req->conn, (argv[0] == (EjsObj*) ejs->trueValue));
    return 0;
}


/*  
    function close(): Void
 */
static EjsObj *req_close(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    ejsSendEvent(ejs, req->emitter, "close", (EjsObj*) req);
    httpCloseConn(req->conn);
    return 0;
}


/*  
    function getResponseHeaders(): Object
 */
static EjsObj *req_getResponseHeaders(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    MprHash     *hp;
    HttpConn    *conn;
    EjsObj      *headers;
    EjsName     n;
    
    conn = req->conn;
    headers = (EjsObj*) ejsCreateSimpleObject(ejs);
    for (hp = 0; (hp = mprGetNextHash(conn->transmitter->headers, hp)) != 0; ) {
        ejsSetPropertyByName(ejs, headers, EN(&n, hp->key), ejsCreateString(ejs, hp->data));
    }
    return (EjsObj*) headers;
}


/*  
    function destroySession(): Void
 */
static EjsObj *req_destroySession(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    ejsDestroySession(ejs, req->server, req->session);
    return 0;
}


/*  
    function finalize(): Void
 */
static EjsObj *req_finalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    httpFinalize(req->conn);
    return 0;
}


/*  
    function read(buffer, offset, count): Number
 */
static EjsObj *req_read(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsByteArray    *ba;
    int             offset, count, nbytes;

    ba = (EjsByteArray*) argv[0];
    offset = (argc >= 1) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 2) ? ejsGetInt(ejs, argv[2]) : -1;

    if (offset < 0) {
        offset = ba->writePosition;
    }
    if (count < 0) {
        count = ba->length - offset;
    }
    if (count < 0) {
        ejsThrowStateError(ejs, "Read count is negative");
        return 0;
    }
    nbytes = httpRead(req->conn, (char*) &ba->value[offset], count);
    if (nbytes < 0) {
        ejsThrowIOError(ejs, "Can't read from socket");
        return 0;
    }
    if (nbytes == 0) {
        if (httpIsEof(req->conn)) {
            return (EjsObj*) ejs->nullValue;
        }
    }
    ba->writePosition += nbytes;
    return (EjsObj*) ejsCreateNumber(ejs, nbytes);
}


#if ES_ejs_web_Request_removeObserver
/*  
    function removeObserver(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_removeObserver(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    ejsRemoveListener(ejs, req->emitter, argv[0], argv[1]);
    return 0;
}
#endif


/*  
    function setHeader(key: String, value: String, overwrite: Boolean = true): Void
 */
static EjsObj *req_setHeader(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    cchar   *key, *value;
    int     overwrite;

    key = ejsGetString(ejs, argv[0]);
    value = ejsGetString(ejs, argv[1]);
    overwrite = argc < 3 || argv[2] == (EjsObj*) ejs->trueValue;
    if (overwrite) {
        httpSetSimpleHeader(req->conn, key, value);
    } else {
        httpAppendHeader(req->conn, key, "%s", value);
    }
    return 0;
}


/*  
    Write text to the client. This call writes the arguments back to the client's browser. 
    function write(data: Object): Void
 */
static EjsObj *req_write(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsString       *s;
    EjsObj          *data;
    EjsByteArray    *ba;
    HttpQueue       *q;
    HttpConn        *conn;
    int             err, len;

    err = 0;
    data = argv[0];
    conn = req->conn;
    q = conn->writeq;

    if (httpIsFinalized(conn)) {
        ejsThrowIOError(ejs, "Response already finalized");
        return 0;
    }

    switch (data->type->id) {
    case ES_String:
        s = (EjsString*) data;
        //  MOB -- this is blocking
        if (httpWriteBlock(q, s->value, s->length, 1) != s->length) {
            err++;
        }
        break;

    case ES_ByteArray:
        ba = (EjsByteArray*) data;
        len = ba->writePosition - ba->readPosition;
        if (httpWriteBlock(q, (char*) &ba->value[ba->readPosition], len, 1) != len) {
            err++;
        }
        break;

    default:
        s = (EjsString*) ejsToString(ejs, data);
        if (s && httpWriteBlock(q, s->value, s->length, 1) != s->length) {
            err++;
        }
    }
    if (ejs->exception) {
        return 0;
    }
    if (err) {
        ejsThrowIOError(ejs, "Can't write to browser");
    }
    return 0;
}


/************************************ Factory *************************************/

//  MOB - standardize on either Copy or Clone
//  MOB -- who is using this?

EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep)
{
    EjsRequest  *newReq;
    HttpConn    *conn;

    newReq = (EjsRequest*) ejsCloneObject(ejs, (EjsObj*) req, deep);
    if (newReq == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    conn = req->conn;
    newReq->conn = conn;
    newReq->ejs = req->ejs;
#if UNUSED
    newReq->pathInfo = conn->receiver->pathInfo;
    newReq->scriptName = conn->receiver->scriptName;
#endif
    newReq->dir = mprStrdup(newReq, req->dir);
    newReq->home = mprStrdup(newReq, req->home);
    newReq->absHome = mprStrdup(newReq, req->absHome);

    //  MOB -- problematic. This is a cross-interp link
    newReq->server = req->server;
    return newReq;
}


EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir)
{
    EjsRequest      *req;
    EjsType         *type;
    HttpReceiver    *rec;
    cchar           *scheme, *ip;
    int             port;

    type = ejsGetTypeByName(ejs, "ejs.web", "Request");
    if ((req = (EjsRequest*) ejsCreate(ejs, type, 0)) == NULL) {
        return 0;
    }
    req->conn = conn;
    req->ejs = ejs;
    req->server = server;
    rec = conn->receiver;
#if UNUSED
    req->dir = mprGetAbsPath(req, dir);
#else
    if (mprIsRelPath(req, dir)) {
        req->dir = mprStrdup(req, dir);
    } else {
        req->dir = mprGetRelPath(req, dir);
    }
#endif
    req->home = makeRelativeHome(ejs, req);
    scheme = conn->secure ? "https" : "http";
    ip = conn->sock ? conn->sock->acceptIp : server->ip;
    port = conn->sock ? conn->sock->acceptPort : server->port;
    
    //  MOB -- should really use a symbolic server name from appweb.conf
    req->absHome = mprAsprintf(req, -1, "%s://%s:%d%s/", scheme, conn->sock->ip, server->port, rec->scriptName);
    return req;
}


/*  
    Mark the object properties for the garbage collector
 */
static void markRequest(Ejs *ejs, EjsRequest *req)
{
    ejsMarkObject(ejs, (EjsObj*) req);
    if (req->cookies) {
        ejsMark(ejs, (EjsObj*) req->cookies);
    }
    if (req->emitter) {
        ejsMark(ejs, (EjsObj*) req->emitter);
    }
    if (req->env) {
        ejsMark(ejs, (EjsObj*) req->env);
    }
    if (req->files) {
        ejsMark(ejs, (EjsObj*) req->files);
    }
    if (req->headers) {
        ejsMark(ejs, (EjsObj*) req->headers);
    }
    if (req->params) {
        ejsMark(ejs, (EjsObj*) req->params);
    }
    if (req->server) {
        ejsMark(ejs, (EjsObj*) req->server);
    }
    if (req->session) {
        ejsMark(ejs, (EjsObj*) req->session);
    }
}


void ejsConfigureRequestType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;
    EjsObj          *prototype;

    type = ejs->requestType = ejsConfigureNativeType(ejs, "ejs.web", "Request", sizeof(EjsRequest));

    helpers = &type->helpers;
    helpers->mark = (EjsMarkHelper) markRequest;
    helpers->clone = (EjsCloneHelper) ejsCloneRequest;
    helpers->getProperty = (EjsGetPropertyHelper) getRequestProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getRequestPropertyCount;
    helpers->getPropertyName = (EjsGetPropertyNameHelper) getRequestPropertyName;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupRequestProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setRequestProperty;

    prototype = type->prototype;
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_observe, (EjsProc) req_observe);
    ejsBindAccess(ejs, prototype, ES_ejs_web_Request_async, (EjsProc) req_async, (EjsProc) req_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_close, (EjsProc) req_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_destroySession, (EjsProc) req_destroySession);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_finalize, (EjsProc) req_finalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_getResponseHeaders, (EjsProc) req_getResponseHeaders);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_read, (EjsProc) req_read);
#if ES_ejs_web_Request_removeObserver
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_removeObserver, (EjsProc) req_removeObserver);
#endif
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_setHeader, (EjsProc) req_setHeader);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_write, (EjsProc) req_write);

#if UNUSED
    for (i = 0; i < ES_ejs_web_Request_NUM_INSTANCE_PROP; i++) {
        if ((trait = ejsGetTrait((EjsObj*) type->prototype, i)) != 0) {
            trait->attributes &= ~EJS_TRAIT_HIDDEN;
        }
    }
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
