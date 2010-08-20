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
 
static int connOk(Ejs *ejs, EjsRequest *req, int throwException)
{
    if (!req->conn || req->conn->rx == 0) {
        if (!ejs->exception && throwException) {
            ejsThrowIOError(ejs, "Connection lost or not established");
        }
        return 0;
    }
    return 1;
}


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
        if (req->conn && (formVars = req->conn->rx->formVars) != 0) {
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
    if (req->conn == 0) {
        return ejs->nullValue;
    }
    if ((cookieHeader = mprLookupHash(req->conn->rx->headers, "cookie")) == 0) {
        req->cookies = (EjsObj*) ejs->nullValue;
    } else {
        argv[0] = (EjsObj*) ejsCreateString(ejs, cookieHeader);
        req->cookies = ejsRunFunctionByName(ejs, ejs->global, ejsName(&n, "ejs.web", "parseCookies"), ejs->global, 1, argv);
    }
    return (EjsObj*) req->cookies;
}


static EjsObj *createEnv(Ejs *ejs, EjsRequest *req)
{
    if (req->env == 0) {
        req->env = ejsCreateSimpleObject(ejs);
    }
    return (EjsObj*) req->env;
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
        if (req->conn == 0) {
            return ejs->nullValue;
        }
        conn = req->conn;
        if (conn->rx->files == 0) {
            return ejs->nullValue;
        }
        req->files = files = (EjsObj*) ejsCreateSimpleObject(ejs);
        for (index = 0, hp = 0; (hp = mprGetNextHash(conn->rx->files, hp)) != 0; index++) {
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


static EjsObj *createHeaders(Ejs *ejs, EjsRequest *req)
{
    MprHash     *hp;
    HttpConn    *conn;
    EjsName     n;
    
    if (req->headers == 0) {
        req->headers = (EjsObj*) ejsCreateSimpleObject(ejs);
        conn = req->conn;
        for (hp = 0; conn && (hp = mprGetNextHash(conn->rx->headers, hp)) != 0; ) {
            ejsSetPropertyByName(ejs, req->headers, EN(&n, hp->key), ejsCreateString(ejs, hp->data));
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
    char        *name, *value;
    int         i, count;
    
    if (req->responseHeaders) {
        ejs = req->ejs;
        count = ejsGetPropertyCount(ejs, req->responseHeaders);
        for (i = 0; i < count; i++) {
            trait = ejsGetTrait(ejs, req->responseHeaders, i);
            if (trait && trait->attributes & 
                    (EJS_TRAIT_HIDDEN | EJS_TRAIT_DELETED | EJS_FUN_INITIALIZER | EJS_FUN_MODULE_INITIALIZER)) {
                continue;
            }
            n = ejsGetPropertyName(ejs, req->responseHeaders, i);
            vp = ejsGetProperty(ejs, req->responseHeaders, i);
            if (n.name && vp && req->conn) {
                if (vp != ejs->nullValue && vp != ejs->undefinedValue) {
                    name = mprStrdup(req->conn, n.name);
                    value = mprStrdup(req->conn, ejsGetString(ejs, vp));
                    httpSetSimpleHeader(req->conn, name, value);
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
    EjsName     n;
    
    if (req->responseHeaders == 0) {
        req->responseHeaders = (EjsObj*) ejsCreateSimpleObject(ejs);
        conn = req->conn;
        if (conn) {
            /* Get default headers */
            for (hp = 0; (hp = mprGetNextHash(conn->tx->headers, hp)) != 0; ) {
                ejsSetPropertyByName(ejs, req->responseHeaders, EN(&n, mprStrdup(req->responseHeaders, hp->key)), 
                    ejsCreateString(ejs, hp->data));
            }
            conn->fillHeaders = (HttpFillHeadersProc) fillResponseHeaders;
            conn->fillHeadersArg = req;
        }
    }
    return (EjsObj*) req->responseHeaders;
}


/*
    This will get the current session or create a new session if required
 */
static EjsSession *getSession(Ejs *ejs, EjsRequest *req, int create)
{
    HttpConn    *conn;

    conn = req->conn;
    if (req->session) {
        return req->session;
    }
    if ((req->session = ejsGetSession(ejs, req)) == NULL && create) {
        req->session = ejsCreateSession(ejs, req, 0, 0);
    }
    if (req->session && conn) {
        httpSetCookie(conn, EJS_SESSION, req->session->id, "/", NULL, 0, conn->secure);
    }
    return req->session;
}


static EjsObj *createString(Ejs *ejs, cchar *value)
{
    if (value == 0) {
        return ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, value);
}


static int getDefaultInt(Ejs *ejs, EjsObj *value, int defaultValue)
{
    if (value == 0 || value == ejs->nullValue) {
        return defaultValue;
    }
    return ejsGetInt(ejs, value);
}


static cchar *getDefaultString(Ejs *ejs, EjsObj *value, cchar *defaultValue)
{
    if (value == 0 || value == ejs->nullValue) {
        return defaultValue;
    }
    return ejsGetString(ejs, value);
}


static cchar *getString(Ejs *ejs, EjsObj *value)
{
    if (value == 0) {
        return "";
    }
    return ejsGetString(ejs, value);
}


static EjsObj *mapNull(Ejs *ejs, EjsObj *value)
{
    if (value == 0) {
        return ejs->nullValue;
    }
    return value;
}


/*
    Get the most "public" host name for the serving host
 */
static cchar *getHost(HttpConn *conn, EjsRequest *req)
{
    cchar       *hostName;

    if (req->server && req->server->name && *req->server->name) {
        hostName = req->server->name;
    } else if (conn && conn->rx->hostName && conn->rx->hostName) {
        hostName = conn->rx->hostName;
    } else if (conn && conn->sock) {
        hostName = conn->sock->acceptIp;
    } else {
        hostName = "localhost";
    }
    return hostName;
}


static EjsObj *getLimits(Ejs *ejs, EjsRequest *req)
{
    if (req->limits == 0) {
        req->limits = ejsCreateSimpleObject(ejs);
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
        return mprStrdup(req, "/");
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
static void *getRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum)
{
    EjsObj      *value;
    EjsName     n;
    HttpConn    *conn;
    cchar       *pathInfo, *scriptName;
    char        *path, *filename, *uri, *ip, *scheme;
    int         port;

    conn = req->conn;

    switch (slotNum) {
    case ES_ejs_web_Request_absHome:
        if (req->conn) {
            if (req->absHome == 0) {
                scheme = conn->secure ? "https" : "http";
                ip = conn->sock ? conn->sock->acceptIp : req->server->ip;
                port = conn->sock ? conn->sock->acceptPort : req->server->port;
                uri = mprAsprintf(req, -1, "%s://%s:%d%s/", scheme, conn->sock->ip, req->server->port, conn->rx->scriptName);
                req->absHome = (EjsObj*) ejsCreateUriAndFree(ejs, uri);
            }
            return req->absHome;
        }
        return ejs->nullValue;

    case ES_ejs_web_Request_authGroup:
        return createString(ejs, conn ? conn->authGroup : NULL);

    case ES_ejs_web_Request_authType:
        return createString(ejs, conn ? conn->authType : NULL);

    case ES_ejs_web_Request_authUser:
        return createString(ejs, conn ? conn->authUser : NULL);

    case ES_ejs_web_Request_autoFinalizing:
        return ejsCreateBoolean(ejs, !req->dontAutoFinalize);

    case ES_ejs_web_Request_config:
        value = ejs->objectType->helpers.getProperty(ejs, (EjsObj*) req, slotNum);
        if (value == (EjsObj*) ejs->nullValue) {
            /* Default to App.config */
            value = ejsGetProperty(ejs, (EjsObj*) ejs->appType, ES_App_config);
        }
        return mapNull(ejs, value);

    case ES_ejs_web_Request_contentLength:
        return ejsCreateNumber(ejs, conn ? conn->rx->length : 0);

    case ES_ejs_web_Request_contentType:
        if (conn) {
            createHeaders(ejs, req);
            return mapNull(ejs, ejsGetPropertyByName(ejs, req->headers, EN(&n, "content-type")));
        } else return ejs->nullValue;

    case ES_ejs_web_Request_cookies:
        if (conn) {
            return createCookies(ejs, req);
        } else return ejs->nullValue;

    case ES_ejs_web_Request_dir:
        return req->dir;

    case ES_ejs_web_Request_env:
        return createEnv(ejs, req);

    case ES_ejs_web_Request_errorMessage:
        return createString(ejs, conn ? conn->errorMsg : NULL);

    case ES_ejs_web_Request_filename:
        if (req->filename == 0 && conn) {
            pathInfo = ejsGetString(ejs, req->pathInfo);
            filename = mprJoinPath(ejs, req->dir->path, &pathInfo[1]);
            req->filename = ejsCreatePathAndFree(ejs, filename);
        }
        return req->filename ? (EjsObj*) req->filename : (EjsObj*) ejs->nullValue;

    case ES_ejs_web_Request_files:
        return createFiles(ejs, req);

    case ES_ejs_web_Request_headers:
        return createHeaders(ejs, req);

    case ES_ejs_web_Request_home:
        if (conn) {
            if (req->home == 0) {
                req->home = ejsCreateUriAndFree(ejs, makeRelativeHome(ejs, req));
            }
            return req->home;
        } else return ejs->nullValue;

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
            req->log = ejsGetProperty(ejs, ejs->appType, ES_App_log);
        }
        return req->log;

    case ES_ejs_web_Request_method:
        return createString(ejs, conn ? conn->rx->method : NULL);

    case ES_ejs_web_Request_originalMethod:
        return mapNull(ejs, req->originalMethod);

    case ES_ejs_web_Request_originalUri:
        if (conn) {
            if (req->originalUri == 0) {
                scheme = (conn->secure) ? "https" : "http";
                /* NOTE: conn->rx->uri is not normalized or decoded */
                req->originalUri = (EjsObj*) ejsCreateFullUri(ejs, scheme, getHost(conn, req), req->server->port, 
                    conn->rx->uri, conn->rx->parsedUri->query, conn->rx->parsedUri->reference, 0);
            }
            return req->originalUri;
        }
        return ejs->nullValue;

    case ES_ejs_web_Request_params:
        return createParams(ejs, req);

    case ES_ejs_web_Request_pathInfo:
        return req->pathInfo;

    case ES_ejs_web_Request_port:
        if (req->port == 0) {
            if (req->server) {
                req->port = (EjsObj*) ejsCreateNumber(ejs, req->server->port);
            } else return ejs->nullValue;
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
        return createString(ejs, conn ? conn->rx->referer: NULL);

    case ES_ejs_web_Request_remoteAddress:
        return createString(ejs, conn ? conn->ip : NULL);

    case ES_ejs_web_Request_responded:
        return ejsCreateBoolean(ejs, req->responded);

    case ES_ejs_web_Request_responseHeaders:
        return createResponseHeaders(ejs, req);

    case ES_ejs_web_Request_scheme:
        if (req->scheme == 0) {
            req->scheme = createString(ejs, (conn && conn->secure) ? "https" : "http");
        }
        return req->scheme;

    case ES_ejs_web_Request_scriptName:
        return req->scriptName;

    case ES_ejs_web_Request_server:
        return req->server;

    case ES_ejs_web_Request_session:
        if (req->session == 0) {
            req->session = getSession(ejs, req, 1);
        }
        return req->session ? (EjsObj*) req->session : (EjsObj*) ejs->nullValue;

    case ES_ejs_web_Request_sessionID:
        if (!req->probedSession) {
            getSession(ejs, req, 0);
            req->probedSession = 1;
        }
        if (req->session) {
            return createString(ejs, req->session->id);
        } else {
            return ejs->nullValue;
        }

    case ES_ejs_web_Request_status:
        if (conn) {
            return ejsCreateNumber(ejs, conn->tx->status);
        } else return ejs->nullValue;

    case ES_ejs_web_Request_uri:
        if (req->uri == 0) {
            scriptName = getDefaultString(ejs, req->scriptName, "");
            if (conn) {
                path = mprStrcat(req, -1, scriptName,
                    getDefaultString(ejs, req->pathInfo, conn->rx->uri), NULL);
                scheme = (conn->secure) ? "https" : "http";
                req->uri = (EjsObj*) ejsCreateFullUri(ejs, 
                    getDefaultString(ejs, req->scheme, scheme),
                    getDefaultString(ejs, req->host, getHost(conn, req)),
                    getDefaultInt(ejs, req->port, req->server->port),
                    path,
                    getDefaultString(ejs, req->query, conn->rx->parsedUri->query),
                    getDefaultString(ejs, req->reference, conn->rx->parsedUri->reference), 0);
            } else {
                path = mprStrcat(req, -1, scriptName, getDefaultString(ejs, req->pathInfo, NULL), NULL);
                req->uri = (EjsObj*) ejsCreateFullUri(ejs, 
                    getDefaultString(ejs, req->scheme, NULL),
                    getDefaultString(ejs, req->host, NULL),
                    getDefaultInt(ejs, req->port, 0),
                    path,
                    getDefaultString(ejs, req->query, NULL),
                    getDefaultString(ejs, req->reference, NULL), 0);
            }
            mprFree(path);
        }
        return req->uri;

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


static int setRequestProperty(Ejs *ejs, EjsRequest *req, int slotNum,  EjsObj *value)
{
    EjsType     *type;
    EjsUri      *up;

    switch (slotNum) {
    default:
    case ES_ejs_web_Request_config:
        return ejs->objectType->helpers.setProperty(ejs, (EjsObj*) req, slotNum, value);

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
        req->host = (EjsObj*) ejsToString(ejs, value);
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
        req->pathInfo = (EjsObj*) ejsToString(ejs, value);
        req->filename = 0;
        req->uri = 0;
        break;

    case ES_ejs_web_Request_port:
        req->port = (EjsObj*) ejsToNumber(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_query:
        req->query = (EjsObj*) ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_reference:
        req->reference = (EjsObj*) ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_responded:
        req->responded = (value == ejs->trueValue);
        break;

    case ES_ejs_web_Request_responseHeaders:
        req->responseHeaders = value;
        break;

    case ES_ejs_web_Request_scriptName:
        req->scriptName = (EjsObj*) ejsToString(ejs, value);
        req->filename = 0;
        req->uri = 0;
        req->absHome = 0;
        break;

    case ES_ejs_web_Request_server:
        type = ejsGetTypeByName(ejs, "ejs.web", "HttpServer");
        if (!ejsIsA(ejs, value, type)) {
            ejsThrowArgError(ejs, "Property is not an instance of HttpServer");
            break;
        }
        req->server = (EjsHttpServer*) value;
        break;

    case ES_ejs_web_Request_scheme:
        req->scheme = (EjsObj*) ejsToString(ejs, value);
        req->uri = 0;
        break;

    case ES_ejs_web_Request_uri:
        up = ejsToUri(ejs, value);
        req->uri = (EjsObj*) up;
        req->filename = 0;
        if (!connOk(ejs, req, 0)) {
            /*
                This is really just for unit testing without a connection
             */
            if (up->uri->scheme) {
                req->scheme = (EjsObj*) ejsCreateString(ejs, up->uri->scheme);
            }
            if (up->uri->host) {
                req->host = (EjsObj*) ejsCreateString(ejs, up->uri->host);
            }
            if (up->uri->port) {
                req->port = (EjsObj*) ejsCreateNumber(ejs, up->uri->port);
            }
            if (up->uri->path) {
                req->pathInfo = (EjsObj*) ejsCreateString(ejs, up->uri->path);
            }
            if (up->uri->query) {
                req->query = (EjsObj*) ejsCreateString(ejs, up->uri->query);
            }
            if (up->uri->reference) {
                req->reference = (EjsObj*) ejsCreateString(ejs, up->uri->reference);
            }
        }
        break;

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
            req->originalMethod = (EjsObj*) ejsCreateString(ejs, req->conn->rx->method);
        }
        req->conn->rx->method = mprStrdup(req, getString(ejs, value));
        break;

    case ES_ejs_web_Request_status:
        if (!connOk(ejs, req, 1)) return 0;
        httpSetStatus(req->conn, getNum(ejs, value));
        req->responded = 1;
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
    return ejs->trueValue;
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *req_set_async(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (argv[0] != ejs->trueValue) {
        ejsThrowIOError(ejs, "Request only supports async mode");
    }
    return 0;
}


/*  
    function autoFinalize(): Void
 */
static EjsObj *req_autoFinalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn && !req->dontAutoFinalize) {
        httpFinalize(req->conn);
    }
    return 0;
}


/*  
    function close(): Void
 */
static EjsObj *req_close(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn) {
        httpFinalize(req->conn);
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
    ejsDestroySession(ejs, req->server, getSession(ejs, req, 0));
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
 */
static EjsObj *req_finalize(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn) {
        httpFinalize(req->conn);
    }
    req->responded = 1;
    return 0;
}


/*  
    function get finalized(): Boolean
 */
static EjsObj *req_finalized(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (req->conn && req->conn->tx) {
        return (EjsObj*) ejsCreateBoolean(ejs, req->conn->tx->finalized);
    }
    return ejs->falseValue;
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
    EjsName     n, qname;
    char        *key;
    int         count, i;

    createHeaders(ejs, req);
    key = (char*) ejsGetString(ejs, argv[0]);

    if ((value = ejsGetPropertyByName(ejs, req->headers, EN(&n, key))) == 0) {
        count = ejsGetPropertyCount(ejs, req->headers);
        for (i = 0; i < count; i++) {
            qname = ejsGetPropertyName(ejs, req->headers, i);
            if (mprStrcmpAnyCase(qname.name, key) == 0) {
                value = ejsGetProperty(ejs, req->headers, i);
                break;
            }
        }
    }
    return (value) ? value : (EjsObj*) ejs->nullValue;
}


/*  
    function observe(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_observe(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    
    conn = req->conn;
    ejsAddObserver(ejs, &req->emitter, argv[0], argv[1]);

    if (conn->readq->count > 0) {
        ejsSendEvent(ejs, req->emitter, "readable", NULL, (EjsObj*) req);
    }
    if (!conn->writeComplete && !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        ejsSendEvent(ejs, req->emitter, "writable", NULL, (EjsObj*) req);
    }
    return 0;
}


/*  
    function read(buffer, offset, count): Number
 */
static EjsObj *req_read(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsByteArray    *ba;
    int             offset, count, nbytes;

    if (!connOk(ejs, req, 1)) return 0;

    ba = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]) : -1;

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
            //  MOB -- should this set req->conn to zero?
            return (EjsObj*) ejs->nullValue;
        }
    }
    ba->writePosition += nbytes;
    return (EjsObj*) ejsCreateNumber(ejs, nbytes);
}


/*  
    function removeObserver(name: [String|Array], listener: Function): Void
 */
static EjsObj *req_removeObserver(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    ejsRemoveObserver(ejs, req->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function sendFile(path: Path): Boolean
 */
static EjsObj *req_sendFile(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsPath         *path;
    HttpConn        *conn;
    HttpRx          *rx;
    HttpTx          *trans;
    HttpPacket      *packet;
    MprPath         info;

    if (!connOk(ejs, req, 1)) return 0;
    conn = req->conn;
    rx = conn->rx;
    trans = conn->tx;

    if (rx->ranges || conn->secure || trans->chunkSize > 0) {
        return ejs->falseValue;
    }
    path = (EjsPath*) argv[0];
    if (mprGetPathInfo(ejs, path->path, &info) < 0) {
        ejsThrowIOError(ejs, "Cannot open %s", path->path);
        return ejs->falseValue;
    }
    httpSetSendConnector(req->conn, path->path);

    packet = httpCreateDataPacket(conn->writeq, 0);
    packet->entityLength = (int) info.size;
    trans->length = trans->entityLength = (int) info.size;
    httpPutForService(conn->writeq, packet, 0);
    httpFinalize(req->conn);
    return ejs->trueValue;
}


/*  
    function setHeader(key: String, value: String, overwrite: Boolean = true): Void
 */
static EjsObj *req_setHeader(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsName     n;
    EjsObj      *old, *value;
    cchar       *key;
    int         overwrite;

    if (!connOk(ejs, req, 1)) return 0;
    key = ejsGetString(ejs, argv[0]);
    value = argv[1];
    overwrite = argc < 3 || argv[2] == (EjsObj*) ejs->trueValue;
    createResponseHeaders(ejs, req);
    if (!overwrite) {
        if ((old = ejsGetPropertyByName(ejs, req->responseHeaders, ejsName(&n, "", key))) != 0) {
            value = (EjsObj*) ejsCreateStringAndFree(ejs, 
                mprAsprintf(req->responseHeaders, -1, "%s, %s", ejsGetString(ejs, old), ejsGetString(ejs, value)));
        }
    }
    ejsSetPropertyByName(ejs, req->responseHeaders, ejsName(&n, "", mprStrdup(req->responseHeaders, key)), value);
    return 0;
}


/*  
    function set limits(limits: Object): Void
 */
static EjsObj *req_setLimits(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    if (!connOk(ejs, req, 1)) return 0;
    if (req->conn->limits == req->server->server->limits) {
        httpSetUniqueConnLimits(req->conn);
    }
    if (req->limits == 0) {
        req->limits = ejsCreateSimpleObject(ejs);
        ejsGetHttpLimits(ejs, req->limits, req->conn->limits, 0);
    }
    ejsBlendObject(ejs, req->limits, argv[0], 1);
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
    ejsSetupTrace(ejs, req->conn, req->conn->trace, argv[0]);
    return 0;
}


/*  
    Write text to the client. This call writes the arguments back to the client's browser. 
    function write(data: Object): Void
 */
static EjsObj *req_write(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsString       *s;
    EjsObj          *data;
    EjsByteArray    *ba;
    HttpQueue       *q;
    HttpConn        *conn;
    int             err, len, written, i;

    if (!connOk(ejs, req, 1)) return 0;

    err = 0;
    written = 0;
    args = (EjsArray*) argv[0];
    conn = req->conn;
    q = conn->writeq;

    if (httpIsFinalized(conn)) {
        ejsThrowIOError(ejs, "Response already finalized");
        return 0;
    }
    for (i = 0; i < args->length; i++) {
        data = args->data[i];
        switch (data->type->id) {
        case ES_String:
            s = (EjsString*) data;
            if ((written = httpWriteBlock(q, s->value, s->length)) != s->length) {
                err++;
            }
            break;

        case ES_ByteArray:
            ba = (EjsByteArray*) data;
            //  MOB -- not updating the read position
            //  MOB ba->readPosition += len;
            //  MOB -- should reset ptrs also
            len = ba->writePosition - ba->readPosition;
            if ((written = httpWriteBlock(q, (char*) &ba->value[ba->readPosition], len)) != len) {
                err++;
            }
            break;

        default:
            s = (EjsString*) ejsToString(ejs, data);
            if (s == NULL || (written = httpWriteBlock(q, s->value, s->length)) != s->length) {
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
    }
    req->responded = 1;

    //  MOB - now
    if (!conn->writeComplete && !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        //  MOB - what if over the queue max
        ejsSendEvent(ejs, req->emitter, "writable", NULL, (EjsObj*) req);
    }
    return 0;
}


/*
    function get written(): Number
 */
static EjsObj *req_written(Ejs *ejs, EjsRequest *req, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, req->written);
}


/************************************ Factory *************************************/
#if UNUSED
EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep)
{
    EjsRequest  *newReq;

    mprAssert(0);
    newReq = (EjsRequest*) ejsCloneObject(ejs, (EjsObj*) req, deep);
    if (newReq == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    newReq->conn = req->conn;
    newReq->ejs = req->ejs;
    newReq->dir = ejsCreatePath(ejs, req->dir->path);

    //  MOB -- should these two be EjsPath
    newReq->home = mprStrdup(newReq, req->home);
    newReq->absHome = mprStrdup(newReq, req->absHome);

    //  MOB -- problematic. This is a cross-interp link
    newReq->server = req->server;
    return newReq;
}
#endif


EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir)
{
    EjsRequest      *req;
    EjsType         *type;
    HttpRx          *rx;

    mprAssert(server);
    mprAssert(conn);
    mprAssert(dir && *dir);

    type = ejsGetTypeByName(ejs, "ejs.web", "Request");
    if (type == NULL || (req = (EjsRequest*) ejsCreate(ejs, type, 0)) == NULL) {
        return 0;
    }
    req->running = 1;
    req->conn = conn;
    req->ejs = ejs;
    req->server = server;
    rx = conn->rx;
    if (mprIsRelPath(req, dir)) {
        req->dir = ejsCreatePath(ejs, dir);
    } else {
        req->dir = ejsCreatePathAndFree(ejs, mprGetRelPath(req, dir));
    }
    req->pathInfo = (EjsObj*) ejsCreateString(ejs, rx->pathInfo);
    req->scriptName = (EjsObj*) ejsCreateString(ejs, rx->scriptName);
    return req;
}


static void destroyRequest(Ejs *ejs, EjsRequest *req)
{
    /* 
        Don't close the connection as it may be in-use by other request objects 
        This is a request close event, not a connection close event 
     */
    //  MOB -- remove this?
}


void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req)
{
    if (!req->closed && req->emitter) {
        req->closed = 1;
        ejsSendEvent(ejs, req->emitter, "close", NULL, (EjsObj*) req);
    }
}


void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req)
{
    if (!req->error && req->emitter) {
        req->error = 1;
        ejsSendEvent(ejs, req->emitter, "error", NULL, (EjsObj*) req);
    }
}


void ejsSendRequestEvent(Ejs *ejs, EjsRequest *req, cchar *event)
{
    if (req->emitter) {
        ejsSendEvent(ejs, req->emitter, event, NULL, (EjsObj*) req);
    }
}


/*  
    Mark the object properties for the garbage collector
 */
static void markRequest(Ejs *ejs, EjsRequest *req)
{
    ejsMarkObject(ejs, req);

    if (req->absHome)           ejsMark(ejs, req->absHome);
    if (req->cookies)           ejsMark(ejs, req->cookies);
    if (req->dir)               ejsMark(ejs, req->dir);
    if (req->emitter)           ejsMark(ejs, req->emitter);
    if (req->env)               ejsMark(ejs, req->env);
    if (req->filename)          ejsMark(ejs, req->filename);
    if (req->files)             ejsMark(ejs, req->files);
    if (req->headers)           ejsMark(ejs, req->headers);
    if (req->home)              ejsMark(ejs, req->home);
    if (req->host)              ejsMark(ejs, req->host);
    if (req->limits)            ejsMark(ejs, req->limits);
    if (req->originalMethod)    ejsMark(ejs, req->originalMethod);
    if (req->originalUri)       ejsMark(ejs, req->originalUri);
    if (req->params)            ejsMark(ejs, req->params);
    if (req->pathInfo)          ejsMark(ejs, req->pathInfo);
    if (req->port)              ejsMark(ejs, req->port);
    if (req->query)             ejsMark(ejs, req->query);
    if (req->reference)         ejsMark(ejs, req->reference);
    if (req->responseHeaders)   ejsMark(ejs, req->responseHeaders);
    if (req->scheme)            ejsMark(ejs, req->scheme);
    if (req->scriptName)        ejsMark(ejs, req->scriptName);
    if (req->server)            ejsMark(ejs, req->server);
    if (req->session)           ejsMark(ejs, req->session);
    if (req->uri)               ejsMark(ejs, req->uri);
} 

void ejsConfigureRequestType(Ejs *ejs)
{
    EjsType         *type;
    EjsTypeHelpers  *helpers;
    EjsObj          *prototype;

    type = ejs->requestType = ejsConfigureNativeType(ejs, "ejs.web", "Request", sizeof(EjsRequest));

    helpers = &type->helpers;
    helpers->mark = (EjsMarkHelper) markRequest;
    helpers->destroy = (EjsDestroyHelper) destroyRequest;
    helpers->getProperty = (EjsGetPropertyHelper) getRequestProperty;
    helpers->getPropertyCount = (EjsGetPropertyCountHelper) getRequestPropertyCount;
    helpers->getPropertyName = (EjsGetPropertyNameHelper) getRequestPropertyName;
    helpers->lookupProperty = (EjsLookupPropertyHelper) lookupRequestProperty;
    helpers->setProperty = (EjsSetPropertyHelper) setRequestProperty;

    prototype = type->prototype;
    ejsBindAccess(ejs, prototype, ES_ejs_web_Request_async, (EjsProc) req_async, (EjsProc) req_set_async);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_autoFinalize, (EjsProc) req_autoFinalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_close, (EjsProc) req_close);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_destroySession, (EjsProc) req_destroySession);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_dontAutoFinalize, (EjsProc) req_dontAutoFinalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_finalize, (EjsProc) req_finalize);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_finalized, (EjsProc) req_finalized);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_flush, (EjsProc) req_flush);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_header, (EjsProc) req_header);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_observe, (EjsProc) req_observe);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_read, (EjsProc) req_read);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_removeObserver, (EjsProc) req_removeObserver);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_sendFile, (EjsProc) req_sendFile);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_setLimits, (EjsProc) req_setLimits);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_setHeader, (EjsProc) req_setHeader);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_trace, (EjsProc) req_trace);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_write, (EjsProc) req_write);
    ejsBindMethod(ejs, prototype, ES_ejs_web_Request_written, (EjsProc) req_written);
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
