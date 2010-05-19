/**
    ejsHttp.c - Http client class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/**************************** Forward Declarations ****************************/

static EjsObj   *getDateHeader(Ejs *ejs, EjsHttp *hp, cchar *key);
static EjsObj   *getStringHeader(Ejs *ejs, EjsHttp *hp, cchar *key);
static int      httpCallback(EjsHttp *hp, MprEvent *event);
static void     httpNotify(HttpConn *conn, int state, int notifyFlags);
static void     prepForm(Ejs *ejs, EjsHttp *hp, char *prefix, EjsObj *data);
static int      readTransfer(Ejs *ejs, EjsHttp *hp, int count);
static EjsObj   *startHttpRequest(Ejs *ejs, EjsHttp *hp, char *method, int argc, EjsObj **argv);
static bool     waitForResponseHeaders(EjsHttp *hp, int timeout);
static bool     waitForState(EjsHttp *hp, int state, int timeout, int throw);
static int      writeHttpData(Ejs *ejs, EjsHttp *hp);

/************************************ Methods *********************************/
/*  
    function Http(uri: Uri = null)
 */
static EjsObj *httpConstructor(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->ejs = ejs;
    hp->conn = httpCreateClient(ejs->http, ejs->dispatcher);
    if (hp->conn == 0) {
        ejsThrowMemoryError(ejs);
    }
    httpSetConnNotifier(hp->conn, httpNotify);
    httpSetConnContext(hp->conn, hp);
    if (argc == 1 && argv[0] != ejs->nullValue) {
        hp->uri = httpUriToString(hp, ((EjsUri*) argv[0])->uri, 1);
    }
    hp->method = mprStrdup(hp, "GET");
    hp->requestContent = mprCreateBuf(hp, HTTP_BUFSIZE, -1);
    hp->responseContent = mprCreateBuf(hp, HTTP_BUFSIZE, -1);
    return (EjsObj*) hp;
}


/*  
    function addListener(name, listener: function): Void
 */
EjsObj *http_addListener(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ejsAddListener(ejs, &hp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function get async(): Boolean
 */
EjsObj *http_async(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return httpGetAsync(hp->conn) ? (EjsObj*) ejs->trueValue : (EjsObj*) ejs->falseValue;
}


/*  
    function set async(enable: Boolean): Void
 */
EjsObj *http_set_async(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    int         async;

    conn = hp->conn;
    async = (argv[0] == (EjsObj*) ejs->trueValue);
    httpSetCallback(conn, (HttpCallback) httpCallback, hp);
    httpSetAsync(conn, async);
    return 0;
}


/*  
    function get available(): Number
    DEPRECATED 1.0.0B3 (11/09)
 */
EjsObj *http_available(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     len;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    len = httpGetContentLength(hp->conn);
    if (len > 0) {
        return (EjsObj*) ejsCreateNumber(ejs, len);
    }
    return (EjsObj*) ejs->minusOneValue;
}


/*  
    function close(): Void
 */
static EjsObj *http_close(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ejsSendEvent(ejs, hp->emitter, "close", (EjsObj*) hp);
    if (hp->conn) {
        httpCloseConn(hp->conn);
        mprFree(hp->conn);
        hp->conn = httpCreateClient(ejs->http, ejs->dispatcher);
        httpSetConnNotifier(hp->conn, httpNotify);
        httpSetConnContext(hp->conn, hp);
    }
    return 0;
}


/*  
    function connect(url = null, data ...): Void
 */
static EjsObj *http_connect(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, NULL, argc, argv);
}


//  TODO
/*  
    function get certificate(): String
 */
static EjsObj *http_certificate(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->certFile) {
        return (EjsObj*) ejsCreateString(ejs, hp->certFile);
    }
    return ejs->nullValue;
}


/*  
    function set setCertificate(value: String): Void
 */
static EjsObj *http_set_certificate(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    mprFree(hp->certFile);
    hp->certFile = mprStrdup(hp, ejsGetString(ejs, argv[0]));
    return 0;
}


/*  
    function get chunkSize(): Boolean
 */
static EjsObj *http_chunkSize(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, httpGetChunkSize(hp->conn));
}


/*  
    function set chunkSize(size: Number): Void
 */
static EjsObj *http_set_chunkSize(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    mprAssert(hp->conn);
    httpSetChunkSize(hp->conn, ejsGetInt(ejs, argv[0]));
    return 0;
}


/*  
    function get contentLength(): Number
 */
static EjsObj *http_contentLength(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     length;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    length = httpGetContentLength(hp->conn);
    return (EjsObj*) ejsCreateNumber(ejs, length);
}


/*  
    function get contentType(): String
 */
static EjsObj *http_contentType(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getStringHeader(ejs, hp, "CONTENT-TYPE");
}


/*  
    function get date(): Date
 */
static EjsObj *http_date(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getDateHeader(ejs, hp, "DATE");
}


/*  
    function del(uri: String = null): Void
 */
static EjsObj *http_del(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "DELETE", argc, argv);
}


/*  
    function get expires(): Date
 */
static EjsObj *http_expires(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getDateHeader(ejs, hp, "EXPIRES");
}


/*  
    function finalize(): Void
 */
static EjsObj *http_finalize(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpFinalize(hp->conn);
    return 0;
}


/*  
    function form(uri: String = null, formData: Object = null): Void
    Issue a POST method with form data
 */
static EjsObj *http_form(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (argc == 2 && argv[1] != ejs->nullValue) {
        httpPrepClientConn(hp->conn, HTTP_NEW_REQUEST);
        mprFlushBuf(hp->requestContent);
        prepForm(ejs, hp, NULL, argv[1]);
        mprAddNullToBuf(hp->requestContent);
        httpSetHeader(hp->conn, "Content-Type", "application/x-www-form-urlencoded");
    }
    return startHttpRequest(ejs, hp, "POST", argc, argv);
}


/*  
    function get followRedirects(): Boolean
 */
static EjsObj *http_followRedirects(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, hp->conn->followRedirects);
}


/*  
    function set followRedirects(flag: Boolean): Void
 */
static EjsObj *http_set_followRedirects(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpFollowRedirects(hp->conn, ejsGetBoolean(ejs, argv[0]));
    return 0;
}


/*  
    function get(uri: String = null, ...data): Void
    The spec allows GET methods to have body data, but is rarely, if ever, used.
 */
static EjsObj *http_get(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "GET", argc, argv);
}


/*  
    Return the (proposed) request headers
    function getRequestHeaders(): Object
 */
static EjsObj *http_getRequestHeaders(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprHash         *p;
    HttpConn        *conn;
    EjsObj          *headers;
    EjsName         n;

    conn = hp->conn;
    headers = (EjsObj*) ejsCreateSimpleObject(ejs);
    for (p = 0; (p = mprGetNextHash(conn->transmitter->headers, p)) != 0; ) {
        ejsSetPropertyByName(ejs, headers, EN(&n, p->key), (EjsObj*) ejsCreateString(ejs, p->data));
    }
    return (EjsObj*) headers;
}


/*  
    function head(uri: String = null): Void
 */
static EjsObj *http_head(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "HEAD", argc, argv);
}


/*  
    function header(key: String): String
 */
static EjsObj *http_header(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsObj   *result;
    char     *str;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    str = (char*) ejsGetString(ejs, argv[0]);
    str = mprStrdup(ejs, str);
    mprStrLower(str);
    result = (EjsObj*) ejsCreateString(ejs, httpGetHeader(hp->conn, str));
    mprFree(str);
    return result;
}


/*  
    function get headers(): Object
 */
static EjsObj *http_headers(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprHashTable    *hash;
    MprHash         *p;
    EjsObj       *results;
    EjsName         qname;
    int             i;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    results = ejsCreateSimpleObject(ejs);
    hash = httpGetHeaderHash(hp->conn);
    if (hash == 0) {
        return (EjsObj*) results;
    }
    for (i = 0, p = mprGetFirstHash(hash); p; p = mprGetNextHash(hash, p), i++) {
        ejsName(&qname, "", p->key);
        ejsSetPropertyByName(ejs, (EjsObj*) results, &qname, (EjsObj*) ejsCreateString(ejs, p->data));
    }
    return (EjsObj*) results;
}


/*  
    function get isSecure(): Boolean
 */
static EjsObj *http_isSecure(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateBoolean(ejs, hp->conn->secure);
}


/*  
    function get key(): String
 */
static EjsObj *http_key(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->keyFile) {
        return (EjsObj*) ejsCreateString(ejs, hp->keyFile);
    }
    return ejs->nullValue;
}


/*  
    function set key(keyFile: String): Void
 */
static EjsObj *http_set_key(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    mprFree(hp->keyFile);
    hp->keyFile = mprStrdup(hp, ejsGetString(ejs, argv[0]));
    return 0;
}


/*  
    function get lastModified(): Date
 */
static EjsObj *http_lastModified(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getDateHeader(ejs, hp, "LAST-MODIFIED");
}


/*  
    function get method(): String
 */
static EjsObj *http_method(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateString(ejs, hp->method);
}


/*  
    function set method(value: String): Void
 */
static EjsObj *http_set_method(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    cchar    *method;

    method = ejsGetString(ejs, argv[0]);
    if (strcmp(method, "DELETE") != 0 && strcmp(method, "GET") != 0 &&  strcmp(method, "HEAD") != 0 &&
            strcmp(method, "OPTIONS") != 0 && strcmp(method, "POST") != 0 && strcmp(method, "PUT") != 0 &&
            strcmp(method, "TRACE") != 0) {
        ejsThrowArgError(ejs, "Unknown HTTP method");
        return 0;
    }
    mprFree(hp->method);
    hp->method = mprStrdup(hp, ejsGetString(ejs, argv[0]));
    return 0;
}


/*  
    function options(uri: String = null, ...data): Void
 */
static EjsObj *http_options(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "OPTIONS", argc, argv);
}


/*  
    function post(uri: String = null, ...requestContent): Void
 */
static EjsObj *http_post(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "POST", argc, argv);
}


/*  
    function put(uri: String = null, form object): Void
 */
static EjsObj *http_put(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "PUT", argc, argv);
}


/*  
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
    Returns a count of bytes read. Non-blocking if a callback is defined. Otherwise, blocks.
 */
static EjsObj *http_read(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    HttpConn        *conn;
    int             offset, count, contentLength;

    conn = hp->conn;
    buffer = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]): -1;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    contentLength = httpGetContentLength(conn);
    if (conn->state >= HTTP_STATE_PARSED && contentLength == hp->readCount) {
        return (EjsObj*) ejs->nullValue;
    }
    if (offset < 0) {
        offset = buffer->writePosition;
    } else if (offset >= buffer->length) {
        ejsThrowOutOfBoundsError(ejs, "Bad read offset value");
        return 0;
    } else {
        ejsSetByteArrayPositions(ejs, buffer, 0, 0);
    }
    if ((count = readTransfer(ejs, hp, count)) < 0) {
        mprAssert(ejs->exception);
        return 0;
    } 
    hp->readCount += count;
    ejsCopyToByteArray(ejs, buffer, buffer->writePosition, (char*) mprGetBufStart(hp->responseContent), count);
    ejsSetByteArrayPositions(ejs, buffer, -1, buffer->writePosition + count);
    mprAdjustBufStart(hp->responseContent, count);
    return (EjsObj*) ejsCreateNumber(ejs, count);
}


/*  
    function readString(count: Number = -1): String
    Read count bytes (default all) of content as a string. This always starts at the first character of content.
 */
static EjsObj *http_readString(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsObj      *result;
    HttpConn    *conn;
    int         count;
    
    count = (argc == 1) ? ejsGetInt(ejs, argv[0]) : -1;
    conn = hp->conn;

    if (!waitForState(hp, HTTP_STATE_CONTENT, conn->async ? 0 : conn->timeout, 0)) {
        return 0;
    }
    if ((count = readTransfer(ejs, hp, count)) < 0) {
        return 0;
    }
    result = (EjsObj*) ejsCreateStringWithLength(ejs, mprGetBufStart(hp->responseContent), count);
    mprAdjustBufStart(hp->responseContent, count);
    return result;
}


/*  
    function removeListener(name, listener: function): Void
 */
EjsObj *http_removeListener(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ejsRemoveListener(ejs, hp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function response(): Stream
 */
static EjsObj *http_response(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->responseCache) {
        return hp->responseCache;
    }
    hp->responseCache = http_readString(ejs, hp, argc, argv);
    return (EjsObj*) hp->responseCache;
}


/*  
    function setCredentials(username: String, password: String): Void
 */
static EjsObj *http_setCredentials(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpSetCredentials(hp->conn, ejsGetString(ejs, argv[0]), ejsGetString(ejs, argv[1]));
    return 0;
}


/*  
    function setHeader(key: String, value: String, overwrite: Boolean = true): Void
 */
EjsObj *http_setHeader(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    cchar   *key, *value;
    bool    overwrite;

    mprAssert(argc >= 2);

    httpPrepClientConn(hp->conn, HTTP_NEW_REQUEST);

    key = ejsGetString(ejs, argv[0]);
    value = ejsGetString(ejs, argv[1]);
    overwrite = (argc == 3) ? ejsGetBoolean(ejs, argv[2]) : 1;
    if (overwrite) {
        httpSetSimpleHeader(hp->conn, key, value);
    } else {
        httpAppendHeader(hp->conn, key, "%s", value);
    }
    return 0;
}


/*  
    function get timeout(): Number
 */
static EjsObj *http_timeout(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateNumber(ejs, hp->conn->timeout);
}


/*  
    function set timeout(value: Number): Void
 */
static EjsObj *http_set_timeout(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpSetTimeout(hp->conn, (int) ejsGetNumber(ejs, argv[0]));
    return 0;
}


/*  
    function trace(uri: String = null, ...data): Void
 */
static EjsObj *http_trace(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "TRACE", argc, argv);
}


/*  
    function get uri(): String
 */
static EjsObj *http_uri(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreateUri(ejs, hp->uri);
}


/*  
    function set uri(newUri: Uri): Void
 */
static EjsObj *http_set_uri(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    mprFree(hp->uri);
    hp->uri = httpUriToString(hp, ((EjsUri*) argv[0])->uri, 1);
    return 0;
}


/*  
    function get status(): Number
 */
static EjsObj *http_status(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     code;

    if (!waitForResponseHeaders(hp, -1)) {
// MOB print("STATUS WAIT RESPONSE FAILED");
        return 0;
    }
    code = httpGetStatus(hp->conn);
    if (code <= 0) {
// MOB print("STATUS CODE %d, state %d", code, hp->conn->state);
        return ejs->nullValue;
    }
    return (EjsObj*) ejsCreateNumber(ejs, code);
}


/*  
    function get statusMessage(): String
 */
static EjsObj *http_statusMessage(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    HttpConn    *conn;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    conn = hp->conn;
    if (conn->errorMsg) {
        return (EjsObj*) ejsCreateString(ejs, conn->errorMsg);
    } else {
        return (EjsObj*) ejsCreateString(ejs, httpGetStatusMessage(hp->conn));
    }
}


/*  
    function wait(timeout: Number = -1): Boolean
    Wait for a request to complete
 */
static EjsObj *http_wait(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprTime     mark;
    int         timeout;

    timeout = (argc == 1) ? ejsGetInt(ejs, argv[0]) : hp->conn->timeout;
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime(ejs);

    if (!waitForState(hp, HTTP_STATE_COMPLETE, timeout, 0)) {
        return (EjsObj*) ejs->falseValue;
    }
    return (EjsObj*) ejs->trueValue;
}


static EjsObj *http_write(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     nbytes;

    hp->data = ejsCreateByteArray(ejs, -1);
    if (ejsWriteToByteArray(ejs, hp->data, 1, &argv[0]) < 0) {
        return 0;
    }
    if ((nbytes = writeHttpData(ejs, hp)) < 0) {
        return 0;
    }
    hp->writeCount += nbytes;
    if (hp->conn->async) {
        if (ejsGetByteArrayAvailable(hp->data) > 0) {
            httpEnableConnEvents(hp->conn);
        }
    }
    return (EjsObj*) ejsCreateNumber(ejs, nbytes);
}


/*********************************** Support **********************************/
/*
    function [get|put|delete|post...](uri = null, ...data): Void
 */
static EjsObj *startHttpRequest(Ejs *ejs, EjsHttp *hp, char *method, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *data;
    EjsNumber       *written;
    EjsUri          *uriObj;
    HttpConn        *conn;
    int             length, nbytes;

    conn = hp->conn;
    hp->responseCache = 0;
    hp->requestContentCount = 0;
    mprFlushBuf(hp->responseContent);

    if (argc >= 1 && argv[0] != ejs->nullValue) {
        mprFree(hp->uri);
        uriObj = (EjsUri*) argv[0];
        hp->uri = httpUriToString(hp, uriObj->uri, 1);
    }
    if (argc == 2 && ejsIsArray(argv[1])) {
        args = (EjsArray*) argv[1];
        if (args->length > 0) {
            data = ejsCreateByteArray(ejs, -1);
            written = ejsWriteToByteArray(ejs, data, 1, &argv[1]);
            mprPutBlockToBuf(hp->requestContent, (char*) data->value, (int) written->value);
            mprAddNullToBuf(hp->requestContent);
            mprAssert(written > 0);
        }
    }
    if (hp->uri == 0) {
        ejsThrowArgError(ejs, "URL is not defined");
        return 0;
    }
    if (method && strcmp(hp->method, method) != 0) {
        mprFree(hp->method);
        hp->method = mprStrdup(hp, method);
    }
    if (hp->method == 0) {
        ejsThrowArgError(ejs, "HTTP Method is not defined");
        return 0;
    }
    if (httpConnect(conn, hp->method, hp->uri) < 0) {
        ejsThrowIOError(ejs, "Can't issue request for \"%s\"", hp->uri);
        return 0;
    }
    if (mprGetBufLength(hp->requestContent) > 0) {
        nbytes = httpWriteBlock(conn->writeq, mprGetBufStart(hp->requestContent), mprGetBufLength(hp->requestContent),
            conn->async ? 0 : 1); 
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Can't write request data for \"%s\"", hp->uri);
            return 0;
        } else if (nbytes > 0) {
            mprAdjustBufStart(hp->requestContent, nbytes);
            hp->requestContentCount += nbytes;
        }
    }
    length = hp->conn->transmitter->length;
    ejsSendEvent(ejs, hp->emitter, "writable", (EjsObj*) hp);
    if (conn->async) {
        httpEnableConnEvents(hp->conn);
    }
    return 0;
}


static void httpNotify(HttpConn *conn, int state, int notifyFlags)
{
    Ejs             *ejs;
    EjsHttp         *hp;

    hp = httpGetConnContext(conn);
    ejs = hp->ejs;

    switch (state) {
    case HTTP_STATE_BEGIN:
        break;

    case HTTP_STATE_PARSED:
        if (hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "headers", (EjsObj*) hp);
        }
        break;

    case HTTP_STATE_CONTENT:
    case HTTP_STATE_PROCESS:
    case HTTP_STATE_RUNNING:
        break;

    case HTTP_STATE_ERROR:
        if (hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "error", (EjsObj*) hp);
        }
        break;

    case HTTP_STATE_COMPLETE:
        if (hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "complete", (EjsObj*) hp);
        }
        break;

    case 0:
        if (hp && hp->emitter) {
            if (notifyFlags & HTTP_NOTIFY_READABLE) {
                ejsSendEvent(ejs, hp->emitter, "readable", (EjsObj*) hp);
            } 
            if (notifyFlags & HTTP_NOTIFY_WRITABLE) {
                ejsSendEvent(ejs, hp->emitter, "writable", (EjsObj*) hp);
            }
        }
        break;
    }
}


/*  
    Read the required number of bytes into the response content buffer. Count < 0 means transfer the entire content.
    Returns the number of bytes read.
 */ 
static int readTransfer(Ejs *ejs, EjsHttp *hp, int count)
{
    MprBuf      *buf;
    HttpConn    *conn;
    int         len, space, nbytes;

    conn = hp->conn;

    //  MOB -- does this block in sync mode?
    buf = hp->responseContent;
    while (count < 0 || mprGetBufLength(buf) < count) {
        len = (count < 0) ? HTTP_BUFSIZE : (count - mprGetBufLength(buf));
        space = mprGetBufSpace(buf);
        if (space < len) {
            mprGrowBuf(buf, len - space);
        }
        if ((nbytes = httpRead(conn, mprGetBufEnd(buf), len)) < 0) {
            ejsThrowIOError(ejs, "Can't read required data");
            return MPR_ERR_CANT_READ;
        }
        mprAdjustBufEnd(buf, nbytes);
        if (hp->conn->async || (nbytes == 0 && conn->state >= HTTP_STATE_COMPLETE)) {
            break;
        }
    }
    if (count < 0) {
        return mprGetBufLength(buf);
    }
    return min(count, mprGetBufLength(buf));
}


/* 
    Write another block of write data
 */
static int writeHttpData(Ejs *ejs, EjsHttp *hp)
{
    EjsByteArray    *ba;
    HttpConn        *conn;
    int             count, nbytes;

    conn = hp->conn;
    if (conn->transmitter->finalized) {
        ejsThrowIOError(ejs, "Can't write to socket");
        return 0;
    }
    ba = hp->data;
    nbytes = 0;
    if (ba && (count = ejsGetByteArrayAvailable(ba)) > 0) {
        nbytes = httpWriteBlock(conn->writeq, (cchar*) &ba->value[ba->readPosition], count, 0);
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Can't write to socket");
            return 0;
        }
        ba->readPosition += nbytes;
    }
    httpServiceQueues(conn);
    return nbytes;
}


/*  
    Respond to an IO event
 */
static int httpCallback(EjsHttp *hp, MprEvent *event)
{
    HttpConn    *conn;
    Ejs         *ejs;

    mprAssert(hp->conn->async);
    conn = hp->conn;
    ejs = hp->ejs;

    //  MOB -- what if this is deleted?
    httpEvent(conn, event);

    if (event->mask & MPR_WRITABLE) {
        if (hp->data) {
            writeHttpData(ejs, hp);
        }
    }
    if (event->mask & MPR_READABLE) {
        if (conn->state >= HTTP_STATE_COMPLETE || 
                (conn->state >= HTTP_STATE_CONTENT && mprGetBufLength(hp->responseContent) > 0)) {
            //  MOB -- remove if notifier works better
            ejsSendEvent(ejs, hp->emitter, "MOBreadable", (EjsObj*) hp);
        }
    } 
    return 0;
}


static EjsObj *getDateHeader(Ejs *ejs, EjsHttp *hp, cchar *key)
{
    MprTime     when;
    cchar       *value;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    value = httpGetHeader(hp->conn, key);
    if (value == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    if (mprParseTime(ejs, &when, value, MPR_UTC_TIMEZONE, NULL) < 0) {
        value = 0;
    }
    return (EjsObj*) ejsCreateDate(ejs, when);
}


static EjsObj *getStringHeader(Ejs *ejs, EjsHttp *hp, cchar *key)
{
    cchar       *value;

    if (!waitForResponseHeaders(hp, -1)) {
        return 0;
    }
    value = httpGetHeader(hp->conn, key);
    if (value == 0) {
        return (EjsObj*) ejs->nullValue;
    }
    return (EjsObj*) ejsCreateString(ejs, value);
}


/*  
    Prepare form data as a series of key-value pairs. Data is formatted according to www-url-encoded specs by 
    mprSetHttpFormData. Objects are flattened into a one level key/value pairs. Keys can have embedded "." separators.
    E.g.  name=value&address=77%20Park%20Lane
 */
static void prepForm(Ejs *ejs, EjsHttp *hp, char *prefix, EjsObj *data)
{
    EjsName     qname;
    EjsObj      *vp;
    EjsString   *value;
    cchar       *key, *sep;
    char        *encodedKey, *encodedValue, *newPrefix, *newKey;
    int         i, count;

    count = ejsGetPropertyCount(ejs, data);
    for (i = 0; i < count; i++) {
        qname = ejsGetPropertyName(ejs, data, i);
        key = qname.name;

        vp = ejsGetProperty(ejs, data, i);
        if (vp == 0) {
            continue;
        }
        if (ejsGetPropertyCount(ejs, vp) > 0) {
            if (prefix) {
                newPrefix = mprAsprintf(hp, -1, "%s.%s", prefix, qname.name);
                prepForm(ejs, hp, newPrefix, vp);
                mprFree(newPrefix);
            } else {
                prepForm(ejs, hp, (char*) qname.name, vp);
            }
        } else {
            value = ejsToString(ejs, vp);
            sep = (mprGetBufLength(hp->requestContent) > 0) ? "&" : "";
            if (prefix) {
                newKey = mprStrcat(hp, -1, prefix, ".", key, NULL);
                encodedKey = mprUriEncode(hp, newKey, MPR_ENCODE_URI_COMPONENT); 
                mprFree(newKey);
            } else {
                encodedKey = mprUriEncode(hp, key, MPR_ENCODE_URI_COMPONENT);
            }
            encodedValue = mprUriEncode(hp, value->value, MPR_ENCODE_URI_COMPONENT);
            mprPutFmtToBuf(hp->requestContent, "%s%s=%s", sep, encodedKey, encodedValue);
            mprFree(encodedKey);
            mprFree(encodedValue);
        }
    }
}


/*  
    Mark the object properties for the garbage collector
 */
void markHttp(Ejs *ejs, EjsHttp *http)
{
    //  MOB -- not needed
    ejsMarkObject(ejs, (EjsObj*) http);

    if (http->responseCache) {
        ejsMark(ejs, (EjsObj*) http->responseCache);
    }
    if (http->emitter) {
        ejsMark(ejs, (EjsObj*) http->emitter);
    }
    if (http->data) {
        ejsMark(ejs, (EjsObj*) http->data);
    }
}


/*  
    Wait for the connection to acheive a requested state
 */
static bool waitForState(EjsHttp *hp, int state, int timeout, int throw)
{
    Ejs             *ejs;
    MprTime         mark;
    HttpConn        *conn;
    HttpUri         *uri;
    char            *url;
    int             count, redirectCount, success, rc;

    mprAssert(state >= HTTP_STATE_PARSED);

    ejs = hp->ejs;
    conn = hp->conn;
    mprAssert(conn->state >= HTTP_STATE_STARTED);

    if  (conn->state >= state) {
        return 1;
    }
    if (conn->state < HTTP_STATE_STARTED) {
        return 0;
    }
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime(hp);
    redirectCount = 0;
    success = count = 0;
    httpFinalize(conn);

    while (conn->state < state && count < conn->retries && redirectCount < 16 && !ejs->exiting && !mprIsExiting(conn) && 
            mprGetElapsedTime(hp, mark) < timeout) {
        count++;
        if ((rc = httpWait(conn, HTTP_STATE_PARSED, timeout)) == 0) {
            if (httpNeedRetry(conn, &url)) {
                if (url) {
                    mprFree(hp->uri);
                    uri = httpCreateUri(hp, url, 0);
                    hp->uri = httpUriToString(hp, uri, 1);
                    httpPrepClientConn(conn, HTTP_NEW_REQUEST);
                }
                count--; 
                redirectCount++;
            } else if (httpWait(conn, state, timeout) == 0) {
                success = 1;
                break;
            }
        } else {
            if (rc == MPR_ERR_CONNECTION) {
                httpFormatError(conn, HTTP_CODE_COMMS_ERROR, "Connection error");
            } else if (rc == MPR_ERR_TIMEOUT) {
                httpFormatError(conn, HTTP_CODE_REQUEST_TIME_OUT, "Request timed out");
            } else {
                httpFormatError(conn, HTTP_CODE_CLIENT_ERROR, "Client request error");
            }
            break;
        }
        if (conn->status == HTTP_CODE_REQUEST_TOO_LARGE || conn->status == HTTP_CODE_REQUEST_URL_TOO_LARGE) {
            /* No point retrying */
            break;
        }
        if (hp->writeCount > 0) {
            /* Can't auto-retry with manual writes */
            break;
        }
        if (hp->requestContentCount > 0) {
            mprAdjustBufStart(hp->requestContent, -hp->requestContentCount);
        }
        /* Force a new connection */
        if (conn->receiver == 0 || conn->receiver->status != HTTP_CODE_UNAUTHORIZED) {
            httpSetKeepAliveCount(conn, -1);
        }
        httpPrepClientConn(conn, HTTP_RETRY_REQUEST);
        if (startHttpRequest(ejs, hp, NULL, 0, NULL) < 0) {
            return 0;
        }
        httpFinalize(conn);
    }
    if (!success) {
        if (throw && ejs->exception == 0) {
            ejsThrowIOError(ejs, "Http request failed: %s", (conn->errorMsg) ? conn->errorMsg : "");
        }
        return 0;
    }
    return 1;
}


/*  
    Wait till the response headers have been received. Safe in sync and async mode. Async mode never blocks.
 */
static bool waitForResponseHeaders(EjsHttp *hp, int timeout)
{
    mprAssert(hp->conn->state >= HTTP_STATE_STARTED);

    if (hp->conn->async) {
        timeout = 0;
    }
    if (timeout < 0) {
        timeout = hp->conn->timeout;
    }
    if (hp->conn->state < HTTP_STATE_STARTED) {
// MOB print("WFR state %d, return zero", hp->conn->state);
        return 0;
    }
    if (hp->conn->state < HTTP_STATE_PARSED && !waitForState(hp, HTTP_STATE_PARSED, timeout, 1)) {
// MOB print("WFR2 state %d, return zero", hp->conn->state);
        return 0;
    }
    return 1;
}


/*
    Need a destructor because the connection object is owned by ejs->http and won't be automatically freed.
 */
static void destroyHttp(Ejs *ejs, EjsHttp *hp)
{
    mprAssert(hp);

    mprFree(hp->conn);
    hp->conn = 0;
    ejsFreeVar(ejs, (EjsObj*) hp, -1);
}


/*********************************** Factory **********************************/

void ejsConfigureHttpType(Ejs *ejs)
{
    EjsType     *type;
    EjsObj      *prototype;

    type = ejsConfigureNativeType(ejs, EJS_EJS_NAMESPACE, "Http", sizeof(EjsHttp));
    type->needFinalize = 1;
    prototype = type->prototype;

    type->helpers.mark = (EjsMarkHelper) markHttp;
    type->helpers.destroy = (EjsDestroyHelper) destroyHttp;

    ejsBindConstructor(ejs, type, (EjsProc) httpConstructor);
    ejsBindMethod(ejs, prototype, ES_Http_addListener, (EjsProc) http_addListener);
    ejsBindAccess(ejs, prototype, ES_Http_async, (EjsProc) http_async, (EjsProc) http_set_async);
    ejsBindMethod(ejs, prototype, ES_Http_available, (EjsProc) http_available);
    ejsBindAccess(ejs, prototype, ES_Http_chunkSize, (EjsProc) http_chunkSize, (EjsProc) http_set_chunkSize);
    ejsBindMethod(ejs, prototype, ES_Http_close, (EjsProc) http_close);
    ejsBindMethod(ejs, prototype, ES_Http_connect, (EjsProc) http_connect);
    ejsBindAccess(ejs, prototype, ES_Http_certificate, (EjsProc) http_certificate, (EjsProc) http_set_certificate);
    ejsBindMethod(ejs, prototype, ES_Http_contentLength, (EjsProc) http_contentLength);
    ejsBindMethod(ejs, prototype, ES_Http_contentType, (EjsProc) http_contentType);
    ejsBindMethod(ejs, prototype, ES_Http_date, (EjsProc) http_date);
    ejsBindMethod(ejs, prototype, ES_Http_del, (EjsProc) http_del);
    ejsBindMethod(ejs, prototype, ES_Http_expires, (EjsProc) http_expires);
    ejsBindMethod(ejs, prototype, ES_Http_finalize, (EjsProc) http_finalize);
    ejsBindAccess(ejs, prototype, ES_Http_followRedirects, (EjsProc) http_followRedirects, 
        (EjsProc) http_set_followRedirects);
    ejsBindMethod(ejs, prototype, ES_Http_form, (EjsProc) http_form);
    ejsBindMethod(ejs, prototype, ES_Http_get, (EjsProc) http_get);
    ejsBindMethod(ejs, prototype, ES_Http_getRequestHeaders, (EjsProc) http_getRequestHeaders);
    ejsBindMethod(ejs, prototype, ES_Http_head, (EjsProc) http_head);
    ejsBindMethod(ejs, prototype, ES_Http_header, (EjsProc) http_header);
    ejsBindMethod(ejs, prototype, ES_Http_headers, (EjsProc) http_headers);
    ejsBindMethod(ejs, prototype, ES_Http_isSecure, (EjsProc) http_isSecure);
    ejsBindAccess(ejs, prototype, ES_Http_key, (EjsProc) http_key, (EjsProc) http_set_key);
    ejsBindMethod(ejs, prototype, ES_Http_lastModified, (EjsProc) http_lastModified);
    ejsBindAccess(ejs, prototype, ES_Http_method, (EjsProc) http_method, (EjsProc) http_set_method);
    ejsBindMethod(ejs, prototype, ES_Http_post, (EjsProc) http_post);
    ejsBindMethod(ejs, prototype, ES_Http_put, (EjsProc) http_put);
    ejsBindMethod(ejs, prototype, ES_Http_read, (EjsProc) http_read);
    ejsBindMethod(ejs, prototype, ES_Http_readString, (EjsProc) http_readString);
    ejsBindMethod(ejs, prototype, ES_Http_removeListener, (EjsProc) http_removeListener);
    ejsBindMethod(ejs, prototype, ES_Http_response, (EjsProc) http_response);
    ejsBindMethod(ejs, prototype, ES_Http_options, (EjsProc) http_options);
    ejsBindMethod(ejs, prototype, ES_Http_setCredentials, (EjsProc) http_setCredentials);
    ejsBindMethod(ejs, prototype, ES_Http_setHeader, (EjsProc) http_setHeader);
    ejsBindMethod(ejs, prototype, ES_Http_status, (EjsProc) http_status);
    ejsBindMethod(ejs, prototype, ES_Http_statusMessage, (EjsProc) http_statusMessage);
    ejsBindAccess(ejs, prototype, ES_Http_timeout, (EjsProc) http_timeout, (EjsProc) http_set_timeout);
    ejsBindMethod(ejs, prototype, ES_Http_trace, (EjsProc) http_trace);
    ejsBindAccess(ejs, prototype, ES_Http_uri, (EjsProc) http_uri, (EjsProc) http_set_uri);
    ejsBindMethod(ejs, prototype, ES_Http_write, (EjsProc) http_write);
    ejsBindMethod(ejs, prototype, ES_Http_wait, (EjsProc) http_wait);
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
