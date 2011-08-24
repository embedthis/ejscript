/*
    ejsRequest.c -- Ejscript web framework.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ejsCompiler.h"
#include    "ejsWeb.h"
#include    "ejs.web.slots.h"

/********************************** Forwards **********************************/

static void defineParam(Ejs *ejs, EjsObj *params, cchar *key, cchar *value);

/************************************* Code ***********************************/
 
static int connOk(Ejs *ejs, EjsRequest *req, int throwException)
{
    if (!req->conn || req->conn->rx == 0) {
        if (!ejs->exception && throwException) {
            ejsThrowString(ejs, "Connection lost or not established");
        }
        return 0;
    }
    return 1;
}


static void defineParam(Ejs *ejs, EjsObj *params, cchar *key, cchar *svalue)
{
    EjsName     qname;
    EjsAny      *value;
    EjsObj      *vp;
    char        *subkey, *end;
    int         slotNum;

    mprAssert(params);

    if (*svalue == '[') {
        value = ejsDeserialize(ejs, ejsCreateStringFromAsc(ejs, svalue));
    } else {
        value = ejsCreateStringFromAsc(ejs, svalue);
    }
    /*  
        name.name.name
     */
    if (strchr(key, '.') == 0) {
        qname = ejsName(ejs, "", key);
        ejsSetPropertyByName(ejs, params, qname, value);

    } else {
        subkey = sclone(key);
        for (end = strchr(subkey, '.'); end; subkey = end, end = strchr(subkey, '.')) {
            *end++ = '\0';
            qname = ejsName(ejs, "", subkey);
            vp = ejsGetPropertyByName(ejs, params, qname);
            if (vp == 0) {
                slotNum = ejsSetPropertyByName(ejs, params, qname, ejsCreateEmptyPot(ejs));
                vp = ejsGetProperty(ejs, params, slotNum);
            }
            params = vp;
        }
        mprAssert(params);
        qname = ejsName(ejs, "", subkey);
        ejsSetPropertyByName(ejs, params, qname, value);
    }
}


#if UNUSED
static int sortForm(MprHash **h1, MprHash **h2)
{
    return scmp((*h1)->key, (*h2)->key);
}


/*
    Create the formData string. This is a stable, sorted string of form variables
 */
static EjsString *createFormData(Ejs *ejs, EjsRequest *req)
{
    MprHashTable    *formVars;
    MprHash         *hp;
    MprList         *list;
    char            *buf, *cp;
    ssize           len;
    int             next;

    if (req->formData == 0) {
        if (req->conn && (formVars = req->conn->rx->formVars) != 0) {
            if ((list = mprCreateList(mprGetHashLength(formVars), 0)) != 0) {
                len = 0;
                for (hp = 0; (hp = mprGetNextKey(formVars, hp)) != NULL; ) {
                    mprAddItem(list, hp);
                    len += slen(hp->key) + slen(hp->data) + 2;
                }
                if ((buf = mprAlloc(len + 1)) != 0) {
                    mprSortList(list, sortForm);
                    cp = buf;
                    for (next = 0; (hp = mprGetNextItem(list, &next)) != 0; ) {
                        strcpy(cp, hp->key); cp += slen(hp->key);
                        *cp++ = '=';
                        strcpy(cp, hp->data); cp += slen(hp->data);
                        *cp++ = '&';
                    }
                    cp[-1] = '\0';
                    req->formData = ejsCreateStringFromAsc(ejs, buf);
                }
            }
        }
    }
    return req->formData;
}
#else

static EjsString *createFormData(Ejs *ejs, EjsRequest *req)
{
    return ejsCreateStringFromAsc(ejs, httpGetFormData(req->conn));
}
#endif


static EjsObj *createParams(Ejs *ejs, EjsRequest *req)
{
    EjsObj          *params;
    MprHashTable    *formVars;
    MprHash         *hp;

    if ((params = req->params) == 0) {
        params = (EjsObj*) ejsCreateEmptyPot(ejs);
        if (req->conn && (formVars = req->conn->rx->formVars) != 0) {
            hp = 0;
            while ((hp = mprGetNextKey(formVars, hp)) != NULL) {
                defineParam(ejs, params, hp->key, hp->data);
            }
        }
    }
    return req->params = params;
}


static EjsObj *createCookies(Ejs *ejs, EjsRequest *req)
{
    EjsObj      *argv[1];
    cchar       *cookieHeader;

    if (req->cookies) {
        return req->cookies;
    }
    if (req->conn == 0) {
        return ESV(null);
    }
    if ((cookieHeader = mprLookupKey(req->conn->rx->headers, "cookie")) == 0) {
        req->cookies = ESV(null);
    } else {
        argv[0] = (EjsObj*) ejsCreateStringFromAsc(ejs, cookieHeader);
        req->cookies = ejsRunFunctionByName(ejs, ejs->global, N("ejs.web", "parseCookies"), ejs->global, 1, argv);
    }
    return req->cookies;
}


static EjsObj *createEnv(Ejs *ejs, EjsRequest *req)
{
    if (req->env == 0) {
        req->env = ejsCreateEmptyPot(ejs);
    }
    return (EjsObj*) req->env;
}


static EjsObj *createFiles(Ejs *ejs, EjsRequest *req)
{
    HttpUploadFile  *up;
    HttpConn        *conn;
    EjsObj          *files, *file;
    MprHash         *hp;
    int             index;

    if (req->files == 0) {
        if (req->conn == 0) {
            return ESV(null);
        }
        conn = req->conn;
        if (conn->rx->files == 0) {
            return ESV(null);
        }
        req->files = files = (EjsObj*) ejsCreateEmptyPot(ejs);
        for (index = 0, hp = 0; (hp = mprGetNextKey(conn->rx->files, hp)) != 0; index++) {
            up = (HttpUploadFile*) hp->data;
            file = (EjsObj*) ejsCreateEmptyPot(ejs);
            ejsSetPropertyByName(ejs, file, EN("filename"), ejsCreatePathFromAsc(ejs, up->filename));
            ejsSetPropertyByName(ejs, file, EN("clientFilename"), ejsCreateStringFromAsc(ejs, up->clientFilename));
            ejsSetPropertyByName(ejs, file, EN("contentType"), ejsCreateStringFromAsc(ejs, up->contentType));
            ejsSetPropertyByName(ejs, file, EN("name"), ejsCreateStringFromAsc(ejs, hp->key));
            ejsSetPropertyByName(ejs, file, EN("size"), ejsCreateNumber(ejs, (MprNumber) up->size));
            ejsSetPropertyByName(ejs, files, EN(hp->key), file);
        }
    }
    return (EjsObj*) req->files;
}


static EjsObj *createHeaders(Ejs *ejs, EjsRequest *req)
{
    EjsName     n;
    EjsString   *value;
    EjsObj      *old;
    HttpConn    *conn;
    MprHash     *hp;
    
    if (req->headers == 0) {
        req->headers = (EjsObj*) ejsCreateEmptyPot(ejs);
        conn = req->conn;
        for (hp = 0; conn && (hp = mprGetNextKey(conn->rx->headers, hp)) != 0; ) {
            n = EN(hp->key);
            if ((old = ejsGetPropertyByName(ejs, req->headers, n)) != 0) {
                value = ejsCreateStringFromAsc(ejs, sjoin(ejsToMulti(ejs, old), "; ", hp->data, NULL));
            } else {
                value = ejsCreateStringFromAsc(ejs, hp->data);
            }
            ejsSetPropertyByName(ejs, req->headers, n, value);
        }
    }
    return (EjsObj*) req->headers;
}


/*
    Callback invoked by Http to fill the http header set just before transmitting headers
 */
static int fillResponseHeaders(EjsRequest *req) 
{
    Ejs         *ejs;
    EjsObj      *vp;
    EjsTrait    *trait;
    EjsName     n;
    char        *value;
    int         i, count;
    
    if (req->responseHeaders) {
        ejs = req->ejs;
        count = ejsGetLength(ejs, req->responseHeaders);
        for (i = 0; i < count; i++) {
            trait = ejsGetPropertyTraits(ejs, req->responseHeaders, i);
            if (trait && trait->attributes & 
                    (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
                continue;
            }
            n = ejsGetPropertyName(ejs, req->responseHeaders, i);
            vp = ejsGetProperty(ejs, req->responseHeaders, i);
            if (n.name && vp && req->conn) {
                if (ejsIsDefined(ejs, vp)) {
                    value = ejsToMulti(ejs, vp);
                    httpSetHeaderString(req->conn, ejsToMulti(ejs, n.name), value);
                }
            }
        }
    }
    return 0;
}


static EjsObj *createResponseHeaders(Ejs *ejs, EjsRequest *req)
{
    MprHash     *hp;
    HttpConn    *conn;
    
    if (req->responseHeaders == 0) {
        req->responseHeaders = (EjsObj*) ejsCreateEmptyPot(ejs);
        conn = req->conn;
        if (conn && conn->tx) {
            /* Get default headers */
            for (hp = 0; (hp = mprGetNextKey(conn->tx->headers, hp)) != 0; ) {
                ejsSetPropertyByName(ejs, req->responseHeaders, EN(hp->key), ejsCreateStringFromAsc(ejs, hp->data));
            }
            conn->headersCallback = (HttpHeadersCallback) fillResponseHeaders;
            conn->headersCallbackArg = req;
        }
    }
    return (EjsObj*) req->responseHeaders;
}


static EjsString *getSessionKey(Ejs *ejs, EjsRequest *req)
{
    cchar   *cookies, *cookie;
    char    *id, *cp, *value;
    int     quoted, len;

    if (!req->conn) {
        return 0;
    }
    cookies = httpGetCookies(req->conn);
    for (cookie = cookies; cookie && (value = strstr(cookie, EJS_SESSION)) != 0; cookie = value) {
        value += strlen(EJS_SESSION);
        while (isspace((int) *value) || *value == '=') {
            value++;
        }
        quoted = 0;
        if (*value == '"') {
            value++;
            quoted++;
        }
        for (cp = value; *cp; cp++) {
            if (quoted) {
                if (*cp == '"' && cp[-1] != '\\') {
                    break;
                }
            } else {
                if ((*cp == ',' || *cp == ';') && cp[-1] != '\\') {
                    break;
                }
            }
        }
        len = (int) (cp - value);
        id = mprMemdup(value, len + 1);
        id[len] = '\0';
        return ejsCreateStringFromAsc(ejs, id);
    }
    return 0;
}


/*
    This will get the current session or create a new session if required
 */
static EjsSession *getSession(Ejs *ejs, EjsRequest *req, int create)
{
    HttpConn    *conn;
    EjsString   *key;

    conn = req->conn;
    if (req->probedSession || !conn) {
        return req->session;
    }
    key = getSessionKey(ejs, req);
    if (key || create) {
        req->session = ejsGetSession(ejs, key, conn->limits->sessionTimeout, create);
        if (req->session && !key) {
            //UNICODE
            httpSetCookie(conn, EJS_SESSION, req->session->key->value, "/", NULL, 0, conn->secure);
        }
        req->probedSession = 1;
    }
    return req->session;
}


static EjsString *createString(Ejs *ejs, cchar *value)
{
    if (value == 0) {
        return ESV(null);
    }
    return ejsCreateStringFromAsc(ejs, value);
}


static int getDefaultInt(Ejs *ejs, EjsNumber *value, int defaultValue)
{
    if (value == 0 || ejsIs(ejs, value, Null)) {
        return defaultValue;
    }
    return ejsGetInt(ejs, value);
}


static cchar *getDefaultString(Ejs *ejs, EjsString *value, cchar *defaultValue)
{
    if (value == 0 || ejsIs(ejs, value, Null)) {
        return defaultValue;
    }
    return ejsToMulti(ejs, value);
}


static cchar *getRequestString(Ejs *ejs, EjsObj *value)
{
    if (value == 0) {
        return "";
    }
    return ejsToMulti(ejs, value);
}


static EjsAny *mapNull(Ejs *ejs, EjsAny *value)
{
    if (value == 0) {
        return ESV(null);
    }
    return value;
}


/*
    Get the best public host name for the serving host
 */
static cchar *getHost(HttpConn *conn, EjsRequest *req)
{
    cchar       *hostName, *cp;

    if (req->server && req->server->name && *req->server->name) {
        hostName = req->server->name;
    } else if (conn && conn->rx->hostHeader && conn->rx->hostHeader) {
        hostName = conn->rx->hostHeader;
    } else if (conn && conn->sock) {
        hostName = conn->sock->acceptIp;
    } else {
        hostName = "localhost";
    }
    if ((cp = schr(hostName, ':')) != 0) {
        return snclone(hostName, cp - hostName);
    }
    return hostName;
}


static EjsObj *getLimits(Ejs *ejs, EjsRequest *req)
{
    if (req->limits == 0) {
        req->limits = ejsCreateEmptyPot(ejs);
        if (req->conn) {
            ejsGetHttpLimits(ejs, req->limits, req->conn->limits, 0);
        }
    }
    return req->limits;
}


static char *makeRelativeHome(Ejs *ejs, EjsRequest *req)
{
    HttpRx      *rx;
    cchar       *path, *end, *sp;
    char        *home, *cp;
    int         levels;

    if (req->conn == NULL) {
        return sclone("/");
    }
    rx = req->conn->rx;
    mprAssert(rx->pathInfo);

    path = rx->pathInfo;
    end = &path[strlen(path)];
    for (levels = 1, sp = &path[1]; sp < end; sp++) {
        if (*sp == '/' && sp[-1] != '/') {
            levels++;
        }
    }
    home = mprAlloc(levels * 3 + 2);
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
static EjsAny *getRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum)
{
    EjsAny      *value, *app;
    HttpConn    *conn;
    cchar       *pathInfo, *scriptName;
    char        *path, *filename, *uri, *ip, *scheme;
    int         port;

    conn = req->conn;

    switch (slotNum) {
    case ES_ejs_web_Request_absHome:
        if (req->absHome == 0) {
            if (req->conn) {
                scheme = conn->secure ? "https" : "http";
                ip = conn->sock ? conn->sock->acceptIp : req->server->ip;
                port = conn->sock ? conn->sock->acceptPort : req->server->port;
                uri = mprAsprintf("%s://%s:%d%s/", scheme, ip, port, conn->rx->scriptName);
                req->absHome = (EjsObj*) ejsCreateUriFromMulti(ejs, uri);
            } else {
                req->absHome = ESV(null);
            }
        }
        return req->absHome;

    case ES_ejs_web_Request_authGroup:
        return createString(ejs, conn ? conn->authGroup : NULL);

    case ES_ejs_web_Request_authType:
        return createString(ejs, conn ? conn->authType : NULL);

    case ES_ejs_web_Request_authUser:
        return createString(ejs, conn ? conn->authUser : NULL);

    case ES_ejs_web_Request_autoFinalizing:
        return ejsCreateBoolean(ejs, !req->dontAutoFinalize);

    case ES_ejs_web_Request_config:
        value = EST(Object)->helpers.getProperty(ejs, req, slotNum);
        if (value == 0 || ejsIs(ejs, value, Null)) {
            /* Default to App.config */
            app = ejsGetProperty(ejs, ejs->global, ES_App);
            value = ejsGetProperty(ejs, app, ES_App_config);
            ejsSetProperty(ejs, req, slotNum, value);
        }
        return mapNull(ejs, value);

    case ES_ejs_web_Request_contentLength:
        return ejsCreateNumber(ejs, conn ? (MprNumber) conn->rx->length : 0);

    case ES_ejs_web_Request_contentType:
        if (conn) {
            createHeaders(ejs, req);
            return mapNull(ejs, ejsGetPropertyByName(ejs, req->headers, EN("content-type")));
        } else return ESV(null);

    case ES_ejs_web_Request_cookies:
        if (conn) {
            return createCookies(ejs, req);
        } else return ESV(null);

    case ES_ejs_web_Request_dir:
        return req->dir;

    case ES_ejs_web_Request_env:
        return createEnv(ejs, req);

    case ES_ejs_web_Request_errorMessage:
        return createString(ejs, conn ? conn->errorMsg : NULL);

    case ES_ejs_web_Request_filename:
        if (req->filename == 0) {
            pathInfo = ejsToMulti(ejs, req->pathInfo);
            if (req->dir) {
                filename = mprJoinPath(req->dir->value, &pathInfo[1]);
                req->filename = ejsCreatePathFromAsc(ejs, filename);
            } else {
                req->filename = ejsCreatePathFromAsc(ejs, pathInfo);
            }
        }
        return req->filename ? (EjsObj*) req->filename : ESV(null);

    case ES_ejs_web_Request_files:
        return createFiles(ejs, req);

    case ES_ejs_web_Request_formData:
        return createFormData(ejs, req);

    case ES_ejs_web_Request_headers:
        return createHeaders(ejs, req);

    case ES_ejs_web_Request_home:
        if (req->home == 0) {
            if (conn) {
                req->home = ejsCreateUriFromMulti(ejs, makeRelativeHome(ejs, req));
            } else return ESV(null);
        }
        return req->home;

    case ES_ejs_web_Request_host:
        if (req->host == 0) {
            /* getHost can handle a null conn */
            req->host = createString(ejs, getHost(conn, req));
        }
        return req->host;

    case ES_ejs_web_Request_isSecure:
        return ejsCreateBoolean(ejs, conn ? conn->secure : 0);

    case ES_ejs_web_Request_limits:
        return getLimits(ejs, req);

    case ES_ejs_web_Request_localAddress:
        return createString(ejs, conn ? conn->sock->acceptIp : NULL);

    case ES_ejs_web_Request_log:
        if (req->log == 0) {
            app = ejsGetProperty(ejs, ejs->global, ES_App);
            req->log = ejsGetProperty(ejs, app, ES_App_log);
        }
        return req->log;

    case ES_ejs_web_Request_method:
        return createString(ejs, conn ? conn->rx->method : NULL);

    case ES_ejs_web_Request_originalMethod:
        return mapNull(ejs, req->originalMethod);

    case ES_ejs_web_Request_originalUri:
        if (req->originalUri == 0) {
            if (conn) {
                scheme = (conn->secure) ? "https" : "http";
                /* NOTE: conn->rx->uri is not normalized or decoded */
                req->originalUri = (EjsObj*) ejsCreateUriFromParts(ejs, scheme, getHost(conn, req), req->server->port, 
                    conn->rx->uri, conn->rx->parsedUri->query, conn->rx->parsedUri->reference, 0);
            } else {
                return ESV(null);
            }
        }
        return req->originalUri;

    case ES_ejs_web_Request_params:
        return createParams(ejs, req);

    case ES_ejs_web_Request_pathInfo:
        return req->pathInfo;

    case ES_ejs_web_Request_port:
        if (req->port == 0) {
            if (req->server) {
                req->port = ejsCreateNumber(ejs, req->server->port);
            } else return ESV(null);
        }
        return req->port;

    case ES_ejs_web_Request_protocol:
        return createString(ejs, conn ? conn->protocol : NULL);

    case ES_ejs_web_Request_query:
        if (req->query == 0) {
            req->query = createString(ejs, conn ? conn->rx->parsedUri->query : NULL);
        }
        return req->query;

    case ES_ejs_web_Request_reference:
        if (req->reference == 0) {
            req->reference = createString(ejs, conn ? conn->rx->parsedUri->reference : NULL);
        } 
        return req->reference;

    case ES_ejs_web_Request_referrer:
        return createString(ejs, conn ? conn->rx->referrer: NULL);

    case ES_ejs_web_Request_remoteAddress:
        return createString(ejs, conn ? conn->ip : NULL);

    case ES_ejs_web_Request_responded:
        return ejsCreateBoolean(ejs, conn->tx->responded);

    case ES_ejs_web_Request_responseHeaders:
        return createResponseHeaders(ejs, req);

    case ES_ejs_web_Request_route:
        return mapNull(ejs, req->route);

    case ES_ejs_web_Request_scheme:
        if (req->scheme == 0) {
            req->scheme = createString(ejs, (conn && conn->secure) ? "https" : "http");
        }
        return req->scheme;

    case ES_ejs_web_Request_scriptName:
        return req->scriptName ? req->scriptName : ESV(empty);

    case ES_ejs_web_Request_server:
        return req->server;

    case ES_ejs_web_Request_session:
        if (req->session == 0) {
            req->session = getSession(ejs, req, 1);
        }
        return req->session ? req->session : ESV(null);

    case ES_ejs_web_Request_sessionID:
        getSession(ejs, req, 0);
        return (req->session) ? req->session->key : ESV(null);

    case ES_ejs_web_Request_status:
        if (conn) {
            return ejsCreateNumber(ejs, conn->tx->status);
        } else return ESV(null);

    case ES_ejs_web_Request_uri:
        if (req->uri == 0) {
            scriptName = getDefaultString(ejs, req->scriptName, "");
            if (conn) {
                path = sjoin(scriptName, getDefaultString(ejs, req->pathInfo, conn->rx->uri), NULL);
                scheme = (conn->secure) ? "https" : "http";
                req->uri = ejsCreateUriFromParts(ejs, 
                    getDefaultString(ejs, req->scheme, scheme),
                    getDefaultString(ejs, req->host, getHost(conn, req)),
                    getDefaultInt(ejs, req->port, req->server->port),
                    path,
                    getDefaultString(ejs, req->query, conn->rx->parsedUri->query),
                    getDefaultString(ejs, req->reference, conn->rx->parsedUri->reference), 
                    0);
            } else {
                path = sjoin(scriptName, getDefaultString(ejs, req->pathInfo, NULL), NULL);
                req->uri = ejsCreateUriFromParts(ejs, 
                    getDefaultString(ejs, req->scheme, NULL),
                    getDefaultString(ejs, req->host, NULL),
                    getDefaultInt(ejs, req->port, 0),
                    path,
                    getDefaultString(ejs, req->query, NULL),
                    getDefaultString(ejs, req->reference, NULL), 
                    0);
            }
        }
        return req->uri;

#if ES_ejs_web_Request_writeBuffer
    case ES_ejs_web_Request_writeBuffer:
        return req->writeBuffer;
#endif

    default:
        if (slotNum < req->pot.numProp) {
            return EST(Object)->helpers.getProperty(ejs, req, slotNum);
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
    EjsName     qname;

    qname = ejsGetPropertyName(ejs, TYPE(req)->prototype, slotNum);
    if (qname.name == 0) {
        qname = ejsGetPotPropertyName(ejs, &req->pot, slotNum);
    }
    return qname;
}


static int lookupRequestProperty(Ejs *ejs, EjsRequest *req, EjsName qname)
{
    int slotNum;
    
    slotNum = ejsLookupProperty(ejs, TYPE(req)->prototype, qname);
    if (slotNum < 0) {
        slotNum = ejsLookupPotProperty(ejs, &req->pot, qname);
    }
    return slotNum;
}


static int getNum(Ejs *ejs, EjsObj *vp)
{
    if (!ejsIs(ejs, vp, Number) && (vp = (EjsObj*) ejsToNumber(ejs, vp)) == 0) {
        return 0;
    }
    return (int) ((EjsNumber*) vp)->value;
}


static int setRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum,  EjsObj *value)
{
    EjsType     *type;
    EjsUri      *up;

    switch (slotNum) {
    default:
        return EST(Object)->helpers.setProperty(ejs, req, slotNum, value);

    case ES_ejs_web_Request_config:
        req->config = value;
        break;

    case ES_ejs_web_Request_absHome:
        req->absHome = (EjsObj*) ejsToUri(ejs, value);
        break;

    case ES_ejs_web_Request_autoFinalizing:
        req->dontAutoFinalize = !ejsGetBoolean(ejs, value);
        break;

    case ES_ejs_web_Request_dir:
        req->dir = ejsToPath(ejs, value);
        req->filename = 0;
        break;

    case ES_ejs_web_Request_filename:
        req->filename = ejsToPath(ejs, value);
        break;

    case ES_ejs_web_Request_headers:
        /*
            This updates the cached header set only. The original headers in the http module are unchanged.
         */
        req->headers = value;
        break;

    case ES_ejs_web_Request_home:
        req->home = ejsToUri(ejs, value);
        break;

    case ES_ejs_web_Request_host:
        req->host = ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_log:
        type = ejsGetType(ejs, ES_Logger);
        if (!ejsIsA(ejs, value, type)) {
            ejsThrowArgError(ejs, "Property is not a logger");
            break;
        }
        req->log = value;
        break;

    case ES_ejs_web_Request_pathInfo:
        req->pathInfo = ejsToString(ejs, value);
        req->filename = 0;
        req->uri = 0;
        break;

    case ES_ejs_web_Request_port:
        req->port = ejsToNumber(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_query:
        req->query = ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_reference:
        req->reference = ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_responded:
        req->conn->tx->responded = (value == ESV(true));
        break;

    case ES_ejs_web_Request_responseHeaders:
        req->responseHeaders = value;
        break;

    case ES_ejs_web_Request_route:
        req->route = value;
        break;

    case ES_ejs_web_Request_scriptName:
        req->scriptName = ejsToString(ejs, value);
        req->filename = 0;
        req->uri = 0;
        req->absHome = 0;
        break;

    case ES_ejs_web_Request_server:
        type = ejsGetTypeByName(ejs, N("ejs.web", "HttpServer"));
        if (!ejsIsA(ejs, value, type)) {
            ejsThrowArgError(ejs, "Property is not an instance of HttpServer");
            break;
        }
        req->server = (EjsHttpServer*) value;
        break;

    case ES_ejs_web_Request_scheme:
        req->scheme = ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_uri:
        up = ejsToUri(ejs, value);
        req->uri = up;
        req->filename = 0;
        if (!connOk(ejs, req, 0)) {
            /*
                This is really just for unit testing without a connection
             */
            if (up->uri->scheme) {
                req->scheme = ejsCreateStringFromAsc(ejs, up->uri->scheme);
            }
            if (up->uri->host) {
                req->host = ejsCreateStringFromAsc(ejs, up->uri->host);
            }
            if (up->uri->port) {
                req->port = ejsCreateNumber(ejs, up->uri->port);
            }
            if (up->uri->path) {
                req->pathInfo = ejsCreateStringFromAsc(ejs, up->uri->path);
            }
            if (up->uri->query) {
                req->query = ejsCreateStringFromAsc(ejs, up->uri->query);
            }
            if (up->uri->reference) {
                req->reference = ejsCreateStringFromAsc(ejs, up->uri->reference);
            }
        }
        break;

#if ES_ejs_web_Request_writeBuffer
    case ES_ejs_web_Request_writeBuffer:
        req->writeBuffer = (EjsByteArray*) value;
        break;
#endif

    /*
        Read-only fields
     */
    case ES_ejs_web_Request_authGroup:
    case ES_ejs_web_Request_authType:
    case ES_ejs_web_Request_authUser:
    case ES_ejs_web_Request_contentLength:
    case ES_ejs_web_Request_cookies:
    case ES_ejs_web_Request_env:
    case ES_ejs_web_Request_errorMessage:
    case ES_ejs_web_Request_formData:
    case ES_ejs_web_Request_files:
    case ES_ejs_web_Request_isSecure:
    case ES_ejs_web_Request_limits:
    case ES_ejs_web_Request_localAddress:
    case ES_ejs_web_Request_originalMethod:
    case ES_ejs_web_Request_originalUri:
    case ES_ejs_web_Request_params:
    case ES_ejs_web_Request_protocol:
    case ES_ejs_web_Request_referrer:
    case ES_ejs_web_Request_remoteAddress:
    case ES_ejs_web_Request_session:
    case ES_ejs_web_Request_sessionID:
        ejsThrowReferenceError(ejs, "Property is readonly");
        break;

    /*
        These tests require a connection
     */
    case ES_ejs_web_Request_method:
        if (!connOk(ejs, req, 1)) return 0;
        if (req->originalMethod == 0) {
            req->originalMethod = ejsCreateStringFromAsc(ejs, req->conn->rx->method);
        }
        req->conn->rx->method = sclone(getRequestString(ejs, value));
        break;

    case ES_ejs_web_Request_status:
        if (!connOk(ejs, req, 1)) return 0;
        httpSetStatus(req->conn, getNum(ejs, value));
        break;
    }
    return 0;
}


/******************************************************************************/
/*  
    function get async(): Boolean
 */
static EjsObj *req_async(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    return ESV(true);
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *req_set_async(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (argv[0] != ESV(true)) {
        ejsThrowIOError(ejs, "Request only supports async mode");
    }
    return 0;
}


/*  
    function autoFinalize(): Void

    Auto-finalize the request if dontAutoFinalize has not been set.
 */
static EjsObj *req_autoFinalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    /* If writeBuffer is set, HttpServer is capturning output for caching */
    if (req->conn && !req->dontAutoFinalize) {
        if (!req->writeBuffer) {
            httpFinalize(req->conn);
        }
        req->finalized = 1;
    }
    return 0;
}


/*  
    function close(): Void
 */
static EjsObj *req_close(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn) {
        if (!req->writeBuffer) {
            httpFinalize(req->conn);
        }
        req->finalized = 1;
        httpCloseRx(req->conn);
    }
    ejsSendRequestCloseEvent(ejs, req);
    return 0;
}


/*  
    function destroySession(): Void
 */
static EjsObj *req_destroySession(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsSession  *sp;

    if ((sp = getSession(ejs, req, 0)) != 0) {
        ejsDestroySession(ejs, sp);
    }
    req->probedSession = 0;
    req->session = 0;
    return 0;
}


/*  
    function dontAutoFinalize(): Void
 */
static EjsObj *req_dontAutoFinalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    req->dontAutoFinalize = 1;
    return 0;
}


/*  
    function finalize(): Void

    This routine is idempotent. If using writeBuffers, it will be called multiple times.
 */
static EjsObj *req_finalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn) {
        if (!req->writeBuffer || req->writeBuffer == ESV(null)) {
            httpFinalize(req->conn);
        } else {
            httpSetResponded(req->conn);
        }
    }
    req->finalized = 1;
    return 0;
}


/*  
    function get finalized(): Boolean
 */
static EjsBoolean *req_finalized(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, req->conn == 0 || req->finalized || req->conn->tx->finalized);
}


/*  
    function flush(dir: Number = Stream.WRITE): Void
 */
static EjsObj *req_flush(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    int     dir;

    if (req->conn) {
        dir = (argc == 1) ? ejsGetInt(ejs, argv[0]) : EJS_STREAM_WRITE;
        if (dir & EJS_STREAM_WRITE) {
            httpFlush(req->conn);
        }
    }
    return 0;
}


/*  
    function header(key: String): String
 */
static EjsObj *req_header(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsObj      *value;
    EjsName     qname;
    char        *key;
    int         count, i;

    createHeaders(ejs, req);
    key = (char*) ejsToMulti(ejs, argv[0]);

    if ((value = ejsGetPropertyByName(ejs, req->headers, EN(key))) == 0) {
        count = ejsGetLength(ejs, req->headers);
        for (i = 0; i < count; i++) {
            qname = ejsGetPropertyName(ejs, req->headers, i);
            if (mcasecmp(qname.name->value, key) == 0) {
                value = ejsGetProperty(ejs, req->headers, i);
                break;
            }
        }
    }
    return (value) ? value : ESV(null);
}


/*  
    function off(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_off(Ejs *ejs, EjsRequest *req, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, req->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function on(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_on(Ejs *ejs, EjsRequest *req, int argc, EjsAny **argv)
{
    HttpConn    *conn;
    
    conn = req->conn;
    ejsAddObserver(ejs, &req->emitter, argv[0], argv[1]);

    if (conn->readq->count > 0) {
        ejsSendEvent(ejs, req->emitter, "readable", NULL, req);
    }
    if (!conn->writeComplete && !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        ejsSendEvent(ejs, req->emitter, "writable", NULL, req);
    }
    return 0;
}


/*  
    function read(buffer, offset, count): Number
 */
static EjsNumber *req_read(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsByteArray    *ba;
    ssize           offset, count, nbytes;

    if (!connOk(ejs, req, 1)) return 0;

    ba = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]) : -1;

    ejsResetByteArrayIfEmpty(ejs, ba);
    if (!ejsMakeRoomInByteArray(ejs, ba, count >= 0 ? count : MPR_BUFSIZE)) {
        return 0;
    }
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
    mprAssert(count > 0);
    nbytes = httpRead(req->conn, (char*) &ba->value[offset], count);
    if (nbytes < 0) {
        ejsThrowIOError(ejs, "Can't read from socket");
        return 0;
    }
    if (nbytes == 0) {
        if (httpIsEof(req->conn)) {
            //  TODO -- should this set req->conn to zero?
            return ESV(null);
        }
    }
    ba->writePosition += nbytes;
    return ejsCreateNumber(ejs, (MprNumber) nbytes);
}


/*  
    function setHeader(key: String, value: String, overwrite: Boolean = true): Void
 */
static EjsObj *req_setHeader(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsString   *value;
    EjsObj      *old;
    cchar       *key;
    int         overwrite;

    if (!connOk(ejs, req, 1)) return 0;
    key = ejsToMulti(ejs, argv[0]);
    value = (EjsString*) argv[1];
    overwrite = argc < 3 || argv[2] == ESV(true);
    createResponseHeaders(ejs, req);
    if (!overwrite) {
        if ((old = ejsGetPropertyByName(ejs, req->responseHeaders, EN(key))) != 0) {
            value = ejsSprintf(ejs, "%@, %@", old, value);
        }
    }
    ejsSetPropertyByName(ejs, req->responseHeaders, EN(key), value);

    /* Just until we have filters - to disable chunk filtering */
    if (strcmp(key, "x-chunk-size") == 0) {
        httpSetChunkSize(req->conn, ejsGetInt(ejs, value));
    }
    return 0;
}


/*  
    function set limits(limits: Object): Void
 */
static EjsObj *req_setLimits(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (!connOk(ejs, req, 1)) return 0;
    if (req->conn->limits == req->server->endpoint->limits) {
        httpSetUniqueConnLimits(req->conn);
    }
    if (req->limits == 0) {
        req->limits = ejsCreateEmptyPot(ejs);
        ejsGetHttpLimits(ejs, req->limits, req->conn->limits, 0);
    }
    ejsBlendObject(ejs, req->limits, argv[0], 0, EJS_BLEND_OVERWRITE);
    ejsSetHttpLimits(ejs, req->conn->limits, req->limits, 0);
    if (req->session) {
        ejsSetSessionTimeout(ejs, req->session, req->conn->limits->sessionTimeout);
    }
    return 0;
}


/*  
    function trace(options): Void
 */
static EjsObj *req_trace(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    ejsSetupTrace(ejs, req->conn->trace, argv[0]);
    return 0;
}


/*
    Single channel for all write data
 */
static ssize writeResponseData(Ejs *ejs, EjsRequest *req, cchar *buf, ssize len)
{
    ssize   written;
    
    if (req->writeBuffer && req->writeBuffer != ESV(null)) {
        if ((written = ejsCopyToByteArray(ejs, req->writeBuffer, -1, buf, len)) > 0) {
            //  MOB - need API to do combined write to ByteArray and inc writePosition
            req->writeBuffer->writePosition += written;
        }
        httpSetResponded(req->conn);
        return written;
    } else {
        return httpWriteBlock(req->conn->writeq, buf, len);
    }
}


/*  
    Write text to the client. This call writes the arguments back to the client's browser. 
    This and writeFile are the lowest channel for write data.
 
    function write(data: Object): Number
 */
static EjsNumber *req_write(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsString       *s;
    EjsObj          *data;
    EjsByteArray    *ba;
    HttpConn        *conn;
    ssize           len, written, total;
    int             err, i;

    conn = req->conn;
    if (!connOk(ejs, req, 1) || httpIsFinalized(conn)) {
        return 0;
    }
    err = 0;
    total = written = 0;
    args = (EjsArray*) argv[0];

    for (i = 0; i < args->length; i++) {
        data = args->data[i];
        switch (TYPE(data)->sid) {
        case S_String:
            s = (EjsString*) data;
            if ((written = writeResponseData(ejs, req, s->value, s->length)) != s->length) {
                err++;
            }
            break;

        case S_ByteArray:
            ba = (EjsByteArray*) data;
            len = ba->writePosition - ba->readPosition;
            if ((written = writeResponseData(ejs, req, (char*) &ba->value[ba->readPosition], len)) != len) {
                err++;
            } else {
                ba->readPosition += len;
            }
            break;

        default:
            s = (EjsString*) ejsToString(ejs, data);
            if (s == NULL || (written = writeResponseData(ejs, req, s->value, s->length)) != s->length) {
                err++;
            }
        }
        if (err) {
            ejsThrowIOError(ejs, "%s", conn->errorMsg);
        }
        if (ejs->exception) {
            return 0;
        }
        req->written += written;
        total += written;
    }

    if (!conn->writeComplete && !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        ejsSendEvent(ejs, req->emitter, "writable", NULL, req);
    }
    return ejsCreateNumber(ejs, total);
}


/*  
    function writeFile(path: Path): Boolean

    Note: this bypasses req->writeBuffer
 */
static EjsObj *req_writeFile(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsPath         *path;
    HttpConn        *conn;
    HttpRx          *rx;
    HttpTx          *tx;
    HttpPacket      *packet;
    MprPath         info;

    if (!connOk(ejs, req, 1)) return 0;
    conn = req->conn;
    rx = conn->rx;
    tx = conn->tx;

    if (tx->outputRanges || conn->secure || tx->chunkSize > 0) {
        return ESV(false);
    }
    path = (EjsPath*) argv[0];
    if (mprGetPathInfo(path->value, &info) < 0) {
        ejsThrowIOError(ejs, "Cannot open %s", path->value);
        return ESV(false);
    }
    packet = httpCreateEntityPacket(0, info.size, NULL);
    tx->length = tx->entityLength = info.size;
    httpSetSendConnector(req->conn, path->value);
    httpPutForService(conn->writeq, packet, 0);
    httpFinalize(req->conn);
    req->finalized = 1;
    return ESV(true);
}


/*
    function get written(): Number
 */
static EjsNumber *req_written(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) req->written);
}


/************************************ Factory *************************************/
/*
    Clone the request object into the "ejs" interpreter.
    This does a "minimal" clone for speed.
 */
EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep)
{
    HttpConn    *conn;
    EjsRequest  *nreq;
    EjsType     *type;

    type = ejsGetTypeByName(ejs, N("ejs.web", "Request"));
    if ((nreq = ejsCreatePot(ejs, type, 0)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    conn = req->conn;
    nreq->conn = conn;
    nreq->ejs = ejs;
    nreq->dir = ejsClone(ejs, req->dir, 1);
    nreq->filename = ejsClone(ejs, req->filename, 1);
    nreq->pathInfo = ejsCreateStringFromAsc(ejs, conn->rx->pathInfo);
    nreq->scriptName = ejsCreateStringFromAsc(ejs, conn->rx->scriptName);
    nreq->running = req->running;
    nreq->cloned = req;

    if (req->route) {
        nreq->route = ejsClone(ejs, req->route, 1);
    }
    if (req->config) {
        nreq->config = ejsClone(ejs, req->config, 1);
    }
    if (req->params) {
        nreq->params = ejsClone(ejs, req->params, 1);
    }
    return nreq;
}


EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir)
{
    EjsRequest      *req;
    EjsType         *type;
    HttpRx          *rx;

    mprAssert(server);
    mprAssert(conn);
    mprAssert(dir && *dir);

    type = ejsGetTypeByName(ejs, N("ejs.web", "Request"));
    if (type == NULL || (req = ejsCreateObj(ejs, type, 0)) == NULL) {
        return 0;
    }
    req->running = 1;
    req->conn = conn;
    req->ejs = ejs;
    req->server = server;
    rx = conn->rx;
    if (mprIsRelPath(dir)) {
        req->dir = ejsCreatePathFromAsc(ejs, dir);
    } else {
        req->dir = ejsCreatePathFromAsc(ejs, mprGetRelPath(dir));
    }
    mprAssert(!VISITED(req->dir));
    //  OPT -- why replicate these two
    req->pathInfo = ejsCreateStringFromAsc(ejs, rx->pathInfo);
    req->scriptName = ejsCreateStringFromAsc(ejs, rx->scriptName);
    return req;
}


void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req)
{
    if (!req->closed && req->emitter) {
        req->closed = 1;
        ejsSendEvent(ejs, req->emitter, "close", NULL, req);
    }
}


void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req)
{
    if (!req->error && req->emitter) {
        req->error = 1;
        ejsSendEvent(ejs, req->emitter, "error", NULL, req);
    }
}


void ejsSendRequestEvent(Ejs *ejs, EjsRequest *req, cchar *event)
{
    if (req->emitter) {
        ejsSendEvent(ejs, req->emitter, event, NULL, req);
    }
}


/*  
    Mark the object properties for the garbage collector
 */
static void manageRequest(EjsRequest *req, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ejsManagePot(req, flags);
        mprMark(req->absHome);
        mprMark(req->config);
        mprMark(req->conn);
        mprMark(req->cookies);
        mprMark(req->dir);
        mprMark(req->emitter);
        mprMark(req->env);
        mprMark(req->filename);
        mprMark(req->files);
        mprMark(req->headers);
        mprMark(req->home);
        mprMark(req->host);
        mprMark(req->limits);
        mprMark(req->originalMethod);
        mprMark(req->originalUri);
        mprMark(req->params);
        mprMark(req->pathInfo);
        mprMark(req->port);
        mprMark(req->query);
        mprMark(req->reference);
        mprMark(req->responseHeaders);
        mprMark(req->route);
        mprMark(req->scheme);
        mprMark(req->scriptName);
        mprMark(req->server);
        mprMark(req->session);
        mprMark(req->uri);
        mprMark(req->cloned);
        mprMark(req->writeBuffer);
        mprMark(req->formData);
    }
}


void ejsConfigureRequestType(Ejs *ejs)
{
    EjsType     *type;
    EjsHelpers  *helpers;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs.web", "Request"), sizeof(EjsRequest), manageRequest, 
            EJS_TYPE_DYNAMIC_INSTANCES | EJS_TYPE_POT)) == 0) {
        return;
    }
    helpers = &type->helpers;
    helpers->getProperty = (EjsGetPropertyHelper) getRequestProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getRequestPropertyCount;
    helpers->getPropertyName = (EjsGetPropertyNameHelper) getRequestPropertyName;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupRequestProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setRequestProperty;

    prototype = type->prototype;
    ejsBindAccess(ejs, prototype, ES_ejs_web_Request_async, req_async, req_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_autoFinalize, req_autoFinalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_close, req_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_destroySession, req_destroySession);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_dontAutoFinalize, req_dontAutoFinalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_finalize, req_finalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_finalized, req_finalized);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_flush, req_flush);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_header, req_header);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_off, req_off);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_on, req_on);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_read, req_read);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_setLimits, req_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_setHeader, req_setHeader);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_trace, req_trace);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_write, req_write);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_writeFile, req_writeFile);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_written, req_written);
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
