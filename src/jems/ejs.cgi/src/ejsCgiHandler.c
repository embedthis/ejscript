/*
 *  ejsCgi.c -- CGI web framework handler.
 *
 *  The ejs handler supports the Ejscript web framework for applications using server-side Javascript. 
 *
 *  Copyright (c) All Rights Reserved. See details at the end of the file.
 *
 *  TODO
 *      - Input post data
 *          - How does Ejscript get post data that is not www encoded?
 *      - make more robust against bad input
 *      - return code diagnostics
 */
/********************************** Includes ************************************/

#include    "ejs.h"
#include    "ejsWeb.h"

#if FUTURE
/***************************** Forward Declarations *****************************/

static void copyFile(cchar *url);
static int  createFormData();
static void decodeFormData(cchar *data);
static void error(void *handle, int code, cchar *fmt, ...);
static void emitHeaders();
static void flushOutput(MprBuf *buf);
static int  getPostData();
static int  getRequest();
static int  initControlBlock();
static char *makeDateString(MprPath *sbuf);
static void processRequest();

/*
 *  Control callbacks
 */
static void discardOutput(void *handle);
static void logError(void *handle, cchar *msg);
static cchar *getHeader(void *handle, cchar *key);
static EjsVar *getVar(void *handle, int collection, int field);
static void setHeader(void *handle, bool allowMultiple, cchar *key, cchar *fmt, ...);
static void setStatus(void *handle, int code);
static int setVar(void *handle, int collection, int field, EjsVar *value);
static int writeBlock(void *handle, cchar *buf, int size);

/*
 *  Routines for Request, Response and Host objects
 */
static EjsVar *createHeaders(Ejs *ejs, MprHashTable *table);
static EjsVar *createString(Ejs *ejs, cchar *value);

#if KEEP
static char hex2Char(char* s);
static void descape(char* src);
#endif

/************************************ Locals **********************************/

static int              debug;                      /* Trace request details to /tmp/ejscgi.log */
static Ejs              *ejs;
static Ejs              *master;
static Mpr              *mpr;
static EjsRequest       *req;
static Http             *http;
static MprHashTable     *requestHeaders;
static MprHashTable     *formVars;
static FILE             *debugFile;

/*
 *  Parsed request details
 */
static int              contentLength = -1;
static char             *contentType;
static char             *cookie;
static char             *currentDate;
static char             *documentRoot;
static char             *ext;
static char             *pathInfo;
static char             *pathTranslated;
static char             *query;
static char             *scriptName;
static char             *uri;

/*
 *  Response fields
 */
static int              headersEmitted;
static char             *input;
static char             *responseMsg;
static int              responseCode;
static MprHashTable     *responseHeaders;
static MprBuf           *output;
static MprBuf           *headerOutput;

#if VXWORKS
static char             **ppGlobalEnviron;
#endif
#if WINCE || FREEBSD
static char             **environ;
#endif
#if BLD_DEBUG
static int              dummy;                      /* Mock up a dummy request */
#endif

/************************************* Code ***********************************/

MAIN(ejsCgiMain, int argc, char **argv)
{
    cchar   *searchPath, *argp;
    int     nextArg, err;

    /*
     *  Create the Embedthis Portable Runtime (MPR) and setup a memory failure handler
     */
    mpr = mprCreate(argc, argv, ejsMemoryFailure);
    mprSetAppName(mpr, argv[0], 0, 0);

    if (strcmp(mprGetAppName(mpr), "ejscgi-debug") == 0) {
        debug++;
    }

    if (mprStart(mpr, MPR_START_EVENTS_THREAD) < 0) {
        mprError(mpr, "Can't start mpr services");
        return EJS_ERR;
    }

    for (err = 0, nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }

        if (strcmp(argp, "--debug") == 0 || strcmp(argp, "-d") == 0) {
            debug++;

#if BLD_DEBUG
        } else if (strcmp(argp, "--dummy") == 0) {
            dummy++;
#endif
        } else if (strcmp(argp, "--log") == 0 || strcmp(argp, "-l") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                ejsStartLogging(mpr, argv[++nextArg]);
            }

        } else if (strcmp(argp, "--searchpath") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            mprPrintfError(mpr, "%s %s\n"
                "Copyright (C) Embedthis Software 2003-2011\n"
                "Copyright (C) Michael O'Brien 2003-2011\n",
               BLD_NAME, BLD_VERSION);
            exit(0);

        } else {
            err++;
            break;
        }
    }

    if (err) {
        mprPrintfError(mpr,
            "Usage: %s [options]\n"
            "  Options:\n"
            "  --log logSpec            # Diagnostic trace\n"
            "  --searchpath ejsPath     # Module search path\n"
            "  --version                # Emit the program version information\n\n",
            mpr->name);
        return -1;
    }

    if (initControlBlock() < 0) {
        error(NULL, 0, "Can't initialize control block");
        exit(1);
    }

    //  FAST CGI must update this
    currentDate = makeDateString(0);

    if (getRequest() < 0) {
        error(NULL, 0, "Can't get request");
    } else {
        processRequest();
    }

    if (responseCode && responseMsg) {
        fprintf(stderr, "ejscgi: ERROR: %s\n", responseMsg);
    }
    return 0;
}


/*********************************** Request Processing ***************************/
/*
 *  Create an initialize the Ejscript Web Framework control block
 */
static int initControlBlock() 
{
#if UNUSED
    control = mprAllocZeroed(mpr, sizeof(EjsWebControl));
    if (control == 0) {
        return MPR_ERR_NO_MEMORY;
    }

    /*
     *  These are the callbacks from the web framework into the gateway
     */
    control->discardOutput = discardOutput;
    control->logError = logError;
    control->getHeader = getHeader;
    control->getVar = getVar;
    control->setHeader = setHeader;
    control->setStatus = setStatus;
    control->setVar = setVar;
    control->write = writeBlock;
#endif

    http = httpCreate(mpr);
    if ((master = ejsOpenWebFramework(http)) == 0) {
        return EJS_ERR;
    }
    output = mprCreateBuf(mpr, EJS_CGI_MIN_BUF, EJS_CGI_MAX_BUF);
    headerOutput = mprCreateBuf(mpr, MPR_BUFSIZE, -1);
    if (output == 0 || headerOutput == 0) {
        return EJS_ERR;
    }
    return 0;
}


/*
 *  Get a request and parse environment and request Uri. Given:
 *
 *      http /simple.cgi/abc/def?a=b&c=d
 *      DOCUMENT_ROOT=/Users/mob/hg/appweb/src/server/web
 *      GATEWAY_INTERFACE=CGI/1.1
 *      CONTENT_LENGTH=NNN 
 *      CONTENT_TYPE
 *      PATH_INFO=/abc/def
 *      PATH_TRANSLATED=/Users/mob/hg/appweb/src/server/web/ab/def
 *      QUERY_STRING=a=b&c=d
 *      REMOTE_ADDR=10.0.0.1234
 *      REMOTE_HOST
 *      REMOTE_PORT
 *      REQUEST_URI=/simple.cgi
 *      REQUEST_METHOD=GET
 *      REQUEST_TRANSPORT=http
 *      SCRIPT_FILENAME=/Users/mob/hg/appweb/src/server/web/simple.cgi
 *      SERVER_NAME=127.0.0.1:7777
 *      SERVER_PORT=7777
 *      SERVER_SOFTWARE=Embedthis-Appweb/3.0A.1
 *      SERVER_PROTOCOL=http
 *      HTTP_ACCEPT=
 *      HTTP_COOKIE=
 *      HTTP_REFERRER=
 *      HTTP_CONNECTION=Keep-Alive
 *      HTTP_HOST=127.0.0.1
 *      HTTP_USER_AGENT=Embedthis-http/3.0A.1
 *      HTTPS
 *      HTTP_*
 *      PATH=
 */
static int getRequest() 
{
    char    key[MPR_MAX_STRING], *ep, *cp, *value;
    int     len, i;

    formVars = mprCreateHash(mpr, EJS_CGI_HDR_HASH);
    requestHeaders = mprCreateHash(mpr, EJS_CGI_HDR_HASH);
    responseHeaders = mprCreateHash(mpr, EJS_CGI_HDR_HASH);

    if (debug) {
        debugFile = fopen("/tmp/ejscgi.log", "w+");
    }

    for (i = 0; environ && environ[i]; i++) {
        if ((ep = environ[i]) == 0) {
            continue;
        }
        if ((cp = strchr(ep, '=')) != 0) {
            len = cp - ep;
            mprMemcpy(key, sizeof(key), ep, len);
            key[len] = '\0';
            mprAddHash(requestHeaders, key, ++cp);
            if (debugFile) {
                fprintf(debugFile, "%-20s = %s\n", key, cp);
            }
        }
    }
    if (debugFile) {
        fclose(debugFile);
        debugFile = 0;
    }

#if BLD_DEBUG
    if (dummy) {
        mprAddHash(requestHeaders, "SCRIPT_NAME", mprStrdup(mpr, "/cgi/ejscgi"));
        mprAddHash(requestHeaders, "DOCUMENT_ROOT", mprStrdup(mpr, "/Users/mob/hg/carmen"));
        mprAddHash(requestHeaders, "PATH_TRANSLATED", mprStrdup(mpr, "/Users/mob/hg/carmen"));
        mprAddHash(requestHeaders, "QUERY_STRING", mprStrdup(mpr, "a=b&c=d"));

        mprAddHash(requestHeaders, "PATH_INFO", mprStrdup(mpr, "/carmen/stock/"));
        mprAddHash(requestHeaders, "REQUEST_URI", mprStrdup(mpr, "/cgi-bin/ejscgi/carmen/stock/"));

//        mprAddHash(requestHeaders, "PATH_INFO", mprStrdup(mpr, "/carmen/web/style.css"));
//        mprAddHash(requestHeaders, "REQUEST_URI", mprStrdup(mpr, "/cgi-bin/ejscgi/carmen/web/style.css"));

//        mprAddHash(requestHeaders, "PATH_INFO", mprStrdup(mpr, "/carmen/web/images/banner.jpg"));
//        mprAddHash(requestHeaders, "REQUEST_URI", mprStrdup(mpr, "/cgi-bin/ejscgi/carmen/web/images/banner.jpg"));

    }
#endif

    documentRoot = (char*) mprLookupHash(requestHeaders, "DOCUMENT_ROOT");
    cookie = (char*) mprLookupHash(requestHeaders, "HTTP_COOKIE");
    contentType = (char*) mprLookupHash(requestHeaders, "CONTENT_TYPE");
    query = (char*) mprLookupHash(requestHeaders, "QUERY_STRING");
    pathTranslated = (char*) mprLookupHash(requestHeaders, "PATH_TRANSLATED");
    pathInfo = (char*) mprLookupHash(requestHeaders, "PATH_INFO");
    scriptName = (char*) mprLookupHash(requestHeaders, "SCRIPT_NAME");
    uri = (char*) mprLookupHash(requestHeaders, "REQUEST_URI");

    if (documentRoot == 0 || pathInfo == 0 || scriptName == 0 || uri == 0) {
        error(NULL, 0, "CGI environment not setup correctly");
        return EJS_ERR;
    }

    value = (char*) mprLookupHash(requestHeaders, "CONTENT_LENGTH");
    if (value) {
        contentLength = atoi(value);
    }
    ext = strrchr(uri, '.');

    if (createFormData() < 0) {
        return EJS_ERR;
    }
    return 0;
}


static void processRequest() 
{
    char   *errorMsg;
    int    flags;

    //  MOB -- not right
    if (strncmp(pathInfo, "web/", 4) == 0) {
        copyFile(pathInfo);
        flushOutput(output);
        return;
    }

    /*
     *  Set default response headers
     */
    setHeader(NULL, 0, "Content-Type", "text/html");
    setHeader(NULL, 0, "Last-Modified", currentDate);
    setHeader(NULL, 0, "Cache-Control", "no-cache");

    flags = 0;
    if ((req = ejsCreateWebRequest(rec, master, scriptName, pathInfo, documentRoot)) == 0) {
        error(NULL, 0, "Can't create web request");
        return;
    }
    ejs = req->ejs;

    ejsRunWebRequest(req);
    if (ejs->exception) {
        errorMsg = ejsGetErrorMsg(ejs, 1);
        error(NULL, 0, "%s", errorMsg);
        return;
    }
    flushOutput(output);
}


static void copyFile(cchar *url)
{
    MprFile     *file;
    char        path[MPR_MAX_FNAME], buf[MPR_BUFSIZE], *ext;
    int         len;

    ext = strrchr(url, '.');
    if (ext) {
        ++ext;
        if (strcmp(ext, "htm") == 0 || strcmp(ext, "html") == 0) {
            setHeader(NULL, 0, "Content-Type", "text/html");
        } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/jpeg");
        } else if (strcmp(ext, "png") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/png");
        } else if (strcmp(ext, "gif") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/gif");
        } else if (strcmp(ext, "tiff") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/tiff");
        } else if (strcmp(ext, "ico") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/x-ico");
        } else if (strcmp(ext, "bmp") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/bmp");
        } else if (strcmp(ext, "pdf") == 0) {
            setHeader(NULL, 0, "Content-Type", "image/pdf");
        } else if (strcmp(ext, "txt") == 0) {
            setHeader(NULL, 0, "Content-Type", "text/plain");
        } else if (strcmp(ext, "css") == 0) {
            setHeader(NULL, 0, "Content-Type", "text/css");
        }
    }
    mprSprintf(path, sizeof(path), "%s/%s", documentRoot, url);
    file = mprOpen(mpr, path, O_RDONLY, 0);
    if (file == 0) {
        error(NULL, 0, "Can't open %s", path);
        return;
    }
    while ((len = mprRead(file, buf, sizeof(buf))) > 0) {
        if (writeBlock(req, buf, len) != len) {
            error(NULL, 0, "Can't write data from %s", path);
            return;
        }
    }
}


/****************************** Control Callbacks ****************************/

static void discardOutput(void *handle)
{
    mprFlushBuf(output);
}


#if FUTURE
/*
 *  Define a form variable as an ejs property in the params[] collection. Support a.b.c syntax
 */
static void defineParam(Ejs *ejs, EjsVar *params, cchar *key, cchar *value)
{
    EjsRequest  *req;
    EjsName     qname;
    EjsVar      *vp;
    char        *subkey, *end;
    int         slotNum;

    req = ejsGetHandle(ejs);
    mprAssert(params);

    /*
     *  name.name.name
     */
    if (strchr(key, '.') == 0) {
        ejsName(&qname, "", key);
        ejsSetPropertyByName(ejs, params, &qname, (EjsVar*) ejsCreateString(ejs, value));

    } else {
        subkey = mprStrdup(ejs, key);
        for (end = strchr(subkey, '.'); end; subkey = end, end = strchr(subkey, '.')) {
            *end++ = '\0';
            ejsName(&qname, "", subkey);
            vp = ejsGetPropertyByName(ejs, params, &qname);
            if (vp == 0) {
                slotNum = ejsSetPropertyByName(ejs, params, &qname, (EjsVar*) ejsCreateSimpleObject(ejs));
                vp = ejsGetProperty(ejs, params, slotNum);
            }
            params = vp;
        }
        mprAssert(params);
        ejsName(&qname, "", subkey);
        ejsSetPropertyByName(ejs, params, &qname, (EjsVar*) ejsCreateString(ejs, value));
    }
}


static EjsVar *createParams(Ejs *ejs)
{
    EjsVar          *params;
    MprHashTable    *formVars;
    MprHash         *hp;

    formVars = conn->request->formVars;

    if ((params = req->params) == 0) {
        params = (EjsVar*) ejsCreateSimpleObject(ejs);
    }
    hp = mprGetFirstHash(formVars);
    while (hp) {
        defineParam(ejs, params, hp->key, hp->data);
        hp = mprGetNextHash(formVars, hp);
    }
    return params;
}
#endif


//  TODO - do we need code?
void error(void *handle, int code, cchar *fmt, ...)
{
    va_list args;

    if (responseMsg == 0) {
        if (code == 0) {
            code = 502;
        }
        responseCode = code;
        va_start(args, fmt);
        responseMsg = mprVasprintf(mpr, -1, fmt, args);
        va_end(args);
    }
}


static cchar *getHeader(void *handle, cchar *key)
{
    return (cchar*) mprLookupHash(requestHeaders, key);
}


#if UNUSED
/*
 *  Add a response cookie
 */
static void setCookie(void *handle, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, int lifetime, bool isSecure)
{
    struct tm   tm;
    cchar       *userAgent, *hostName;
    char        dateStr[64], *cp, *expiresAtt, *expires, *domainAtt, *domain, *secure;

    if (path == 0) {
        path = "/";
    }

    userAgent = getHeader(handle, "HTTP_USER_AGENT");
    hostName = getHeader(handle, "HTTP_HOST");

    /*
     *  Fix for Safari and Bonjour addresses with a trailing ".". Safari discards cookies without a domain specifier
     *  or with a domain that includes a trailing ".". Solution: include an explicit domain and trim the trailing ".".
     *
     *   User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10_5_6; en-us) 
     *       AppleWebKit/530.0+ (KHTML, like Gecko) Version/3.1.2 Safari/525.20.1
     */
    if (cookieDomain == 0 && userAgent && strstr(userAgent, "AppleWebKit") != 0) {
        domain = mprStrdup(mpr, hostName);
        if ((cp = strchr(domain, ':')) != 0) {
            *cp = '\0';
        }
        if (*domain && domain[strlen(domain) - 1] == '.') {
            domain[strlen(domain) - 1] = '\0';
        } else {
            domain = 0;
        }
    } else {
        domain = 0;
    }
    if (domain) {
        domainAtt = "; domain=";
    } else {
        domainAtt = "";
    }
    if (lifetime > 0) {
        mprDecodeUniversalTime(mpr, &tm, mprGetTime(mpr) + (lifetime * MPR_TICKS_PER_SEC));
        mprFormatTime(mpr, MPR_RFC_DATE, &tm);
        expiresAtt = "; expires=";
        expires = dateStr;

    } else {
        expires = expiresAtt = "";
    }
    if (isSecure) {
        secure = "; secure";
    } else {
        secure = ";";
    }

    /*
     *  Allow multiple cookie headers. Even if the same name. Later definitions take precedence
     */
    setHeader(handle, 1, "Set-Cookie", 
        mprStrcat(mpr, -1, name, "=", value, "; path=", path, domainAtt, domain, expiresAtt, expires, secure, NULL));
    setHeader(handle, 0, "Cache-control", "no-cache=\"set-cookie\"");
}
#endif


static void logError(void *handle, cchar *msg)
{
    mprLog(handle, 2, "%s", msg);
}


/*
 *  Add a response header
 */
static void setHeader(void *handle, bool allowMultiple, cchar *key, cchar *fmt, ...)
{
    char            *value;
    va_list         vargs;

    va_start(vargs, fmt);
    value = mprVasprintf(mpr, EJS_MAX_HEADERS, fmt, vargs);

    if (allowMultiple) {
        mprAddDuplicateHash(responseHeaders, key, value);
    } else {
        mprAddHash(responseHeaders, key, value);
    }
}


static void setStatus(void *handle, int status)
{
    responseCode = status;
}


static int setVar(void *handle, int collection, int field, EjsVar *value)
{
    //  TODO - currently all fields are read-only
    return EJS_ERR;
}


/*
 *  Write data to the client. Will buffer and flush as required. will create headers if required.
 */
static int writeBlock(void *handle, cchar *buf, int size)
{
    int     len, rc;

    len = mprGetBufLength(output);
    if ((len + size) < EJS_CGI_MAX_BUF) {
        rc = mprPutBlockToBuf(output, buf, size);
    } else {
        flushOutput(output);
        if (size < EJS_CGI_MAX_BUF) {
            rc = mprPutBlockToBuf(output, buf, size);
        } else {
            rc = write(1, (char*) buf, size);
        }
    }
    return rc;
}


/*********************************************************************************/
#if UNUSED
/*
 *  Virtual Host, Request and Response objects. Better to create properties virtually as it is much faster
 */

/*
 *  Get a variable from one of the virtual objects: Host, Request, Response
 */
static EjsVar *getVar(void *handle, int collection, int field)
{
    EjsString   *str;
    char        *s, *cp;

    /*
     *  TODO - should cache some of this instead of recreating each time
     */
    switch (field) {

    case ES_ejs_web_Request_authAcl:
    case ES_ejs_web_Request_authGroup:
    case ES_ejs_web_Request_authUser:
        //  TODO 
        return (EjsVar*) ejs->undefinedValue;

    case ES_ejs_web_Request_authType:
        return createString(ejs, getHeader(handle, "AUTH_TYPE"));

    case ES_ejs_web_Request_dir:
        return (EjsVar*) ejsCreatePath(ejs, ereq->dir);

    case ES_ejs_web_Request_documentRoot:
        return createString(ejs, getHeader(handle, "DOCUMENT_ROOT"));

    case ES_ejs_web_Request_env:
        if (ereq->env == 0) {
            ereq->env = (EjsVar*) ejsCreateSimpleObject(ejs);
        }
        return ereq->env;

#if FUTURE
    case ES_ejs_web_Request_files:
        if (ereq->files == 0) {
            ereq->files = createWebFiles(ejs, req->files);
        }
        return (EjsVar*) ereq->files;
#endif

    case ES_ejs_web_Request_headers:
        if (ereq->headers == 0) {
            ereq->headers = createHeaders(ejs, requestHeaders);
        }
        return (EjsVar*) ereq->headers;

    case ES_ejs_web_Request_hostName:
        return createString(ejs, getHeader(handle, "HTTP_HOST"));

    case ES_ejs_web_Request_method:
        return createString(ejs, getHeader(handle, "REQUEST_METHOD"));

    case ES_ejs_web_Request_originalUri:
        return createString(ejs, getHeader(handle, "REQUEST_URI"));

#if FUTURE
    case ES_ejs_web_Request_params:
        if (ereq->params == 0) {
            ereq->params = createParams(ejs);
        }
        return (EjsVar*) ereq->params;
#endif

    case ES_ejs_web_Request_pathInfo:
        return createString(ejs, getHeader(handle, "PATH_INFO"));

    case ES_ejs_web_Request_query:
        return createString(ejs, getHeader(handle, "QUERY_STRING"));

    case ES_ejs_web_Request_remoteAddress:
        return createString(ejs, getHeader(handle, "REMOTE_ADDRESS"));

    case ES_ejs_web_Request_scheme:
        return createString(ejs, getHeader(handle, "SERVER_PROTOCOL"));

    //  TODO - this is meant to be un-encoded
    case ES_ejs_web_Request_scriptName:
        return createString(ejs, getHeader(handle, "SCRIPT_NAME"));

    case ES_ejs_web_Request_serverName:
        str = (EjsString*) createString(ejs, getHeader(handle, "SERVER_NAME"));
        if ((cp = strchr(str->value, ':')) != 0) {
            *cp = '\0';
            str->length = strlen(str->value);
        }
        return (EjsVar*) str;

    case ES_ejs_web_Request_serverPort:
        if ((s = (char*) getHeader(handle, "SERVER_PORT")) != 0) {
            return (EjsVar*) ejsCreateNumber(ejs, (MprNumber) mprAtoi(s, 10));
        } else {
            return (EjsVar*) ejs->nullValue;
        }

    case ES_ejs_web_Request_sessionID:
        if (ereq->session) {
            return createString(ejs, ereq->session->id);
        } else {
            return ejs->nullValue;
        }

    case ES_ejs_web_Request_software:
        return createString(ejs, "ejs-cgi");

    default: 
        if (ereq->obj.slots) {
            //  TODO - must add this for lookup and set also
            return (ejs->objectType->helpers->getProperty)(ejs, (EjsVar*) ereq, field);
        }
    }
    return (EjsVar*) ejs->nullValue;
}

#endif
/**********************************************************************************/

static int createFormData()
{ 
    char    *pat;

    decodeFormData(query);

    pat = "application/x-www-form-urlencoded";
    if (mprStrcmpAnyCaseCount(contentType, pat, (int) strlen(pat)) == 0) {
        if (getPostData() < 0) {
            return EJS_ERR;
        }
        if (contentLength > 0) {
            pat = "application/x-www-form-urlencoded";
            if (mprStrcmpAnyCaseCount(contentType, pat, (int) strlen(pat)) == 0) {
                decodeFormData(input);
            }
        }
    }
    return 0;
}


/*
 *  Create a string variable
 */
static EjsVar *createString(Ejs *ejs, cchar *value)
{
    if (value == 0) {
        return ejs->nullValue;
    }
    return (EjsVar*) ejsCreateString(ejs, value);
}


/*
 *  A header object from a given hash table
 */  
static EjsVar *createHeaders(Ejs *ejs, MprHashTable *table)
{
    MprHash     *hp;
    EjsVar      *headers, *header;
    EjsName     qname;
    int         index;
    
    headers = (EjsVar*) ejsCreateArray(ejs, mprGetHashCount(table));
    for (index = 0, hp = 0; (hp = mprGetNextHash(table, hp)) != 0; ) {
        header = (EjsVar*) ejsCreateSimpleObject(ejs);
        ejsSetPropertyByName(ejs, header, ejsName(&qname, "", hp->key), (EjsVar*) ejsCreateString(ejs, hp->data));
        ejsSetProperty(ejs, headers, index++, header);
    }
    return headers;
}


/*
 *  Emit all headers
 */
static void emitHeaders()
{
    MprHash     *hp;
    int         len;

    hp = mprGetFirstHash(responseHeaders);
    while (hp) {
        len = strlen(hp->key) + strlen(hp->data) + 4;
        if (mprGetBufSpace(headerOutput) < len) {
            flushOutput(headerOutput);
        }
        mprPutStringToBuf(headerOutput, hp->key);
        mprPutStringToBuf(headerOutput, ": ");
        mprPutStringToBuf(headerOutput, hp->data);
        mprPutStringToBuf(headerOutput, "\r\n");
        hp = mprGetNextHash(responseHeaders, hp);
    }
    mprPutStringToBuf(headerOutput, "\r\n");
    flushOutput(headerOutput);
    headersEmitted = 1;
}


/*
 *  Flush all output and emit headers first if required
 */
static void flushOutput(MprBuf *buf)
{
    int     rc, len;

    if (!headersEmitted && buf != headerOutput) {
        emitHeaders();
    }

    while ((len = mprGetBufLength(buf)) > 0) {
        rc = write(1, mprGetBufStart(buf), len);
        if (rc < 0) {
            //  TODO diag
            return;
        }
        mprAdjustBufStart(buf, rc);
    }
    mprFlushBuf(buf);
}


/*
 *  Decode the query and post form data into formVars
 */
static void decodeFormData(cchar *data)
{
    char    *value, *key, *buf, *decodedKey, *decodedValue;
    int     buflen;

    buf = mprStrdup(mpr, data);
    buflen = strlen(buf);

    /*
     *  Crack the input into name/value pairs 
     */
    for (key = strtok(buf, "&"); key; key = strtok(0, "&")) {
        if ((value = strchr(key, '=')) != 0) {
            *value++ = '\0';
            decodedValue = mprUrlDecode(mpr, value);
        }
        decodedKey = mprUrlDecode(mpr, key);
        mprAddHash(formVars, key, value);
    }
}


/*
 *  Read post data
 */
static int getPostData()
{
    int     bytes, len;

    if (contentLength == 0) {
        return 0;
    }

    input = (char*) malloc(contentLength + 1);
    if (input == 0) {
        error(NULL, 0, "Content length is too large");
        return MPR_ERR_NO_MEMORY;
    }

    for (len = 0; len < contentLength; ) {
        bytes = read(0, &input[len], contentLength - len);
        if (bytes < 0) {
            error(NULL, 0, "Couldn't read CGI input");
            return MPR_ERR_CANT_READ;
        }
        len += bytes;
    }
    input[contentLength] = '\0';
    contentLength = len;
    return 0;
}


#if KEEP
static char hex2Char(char* s) 
{
    char    c;

    if (*s >= 'A') {
        c = (*s & 0xDF) - 'A';
    } else {
        c = *s - '0';
    }
    s++;

    if (*s >= 'A') {
        c = c * 16 + ((*s & 0xDF) - 'A');
    } else {
        c = c * 16 + (*s - '0');
    }
    return c;
}


static void descape(char* src) 
{
    char    *dest;

    dest = src;
    while (*src) {
        if (*src == '%') {
            *dest++ = hex2Char(++src) ;
            src += 2;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}
#endif


/*
 *  Get a date string. If sbuf is non-null, get the modified time of that file. If null, then get the current system time.
 */
static char *makeDateString(MprPath *sbuf)
{
    MprTime     when;
    struct tm   tm;

    if (sbuf == 0) {
        when = mprGetTime(mpr);
    } else {
        when = (MprTime) sbuf->mtime * MPR_TICKS_PER_SEC;
    }

    mprDecodeUniversalTime(mpr, &tm, when);
    return mprFormatTime(mpr, MPR_RFC_DATE, &tm);
}

#else
int main(int argc, char **argv) { return 0; }
#endif /* FUTURE */


/*
 *  @copy   default
 *
 *  Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
 *  Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
 *
 *  This software is distributed under commercial and open source licenses.
 *  You may use the GPL open source license described below or you may acquire
 *  a commercial license from Embedthis Software. You agree to be fully bound
 *  by the terms of either license. Consult the LICENSE.TXT distributed with
 *  this software for full details.
 *
 *  This software is open source; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version. See the GNU General Public License for more
 *  details at: http://www.embedthis.com/downloads/gplLicense.html
 *
 *  This program is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  This GPL license does NOT permit incorporating this software into
 *  proprietary programs. If you are unable to comply with the GPL, you must
 *  acquire a commercial license to use this software. Commercial licenses
 *  for this software and support services are available from Embedthis
 *  Software at http://www.embedthis.com
 *
 *  Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
