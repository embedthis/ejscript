/**
    ejsHttp.c - Http client class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/**************************** Forward Declarations ****************************/

static EjsDate  *getDateHeader(Ejs *ejs, EjsHttp *hp, cchar *key);
static EjsString *getStringHeader(Ejs *ejs, EjsHttp *hp, cchar *key);
static void     httpEvent(HttpConn *conn, MprEvent *event);
static void     httpEventChange(HttpConn *conn, int event, int arg);
static void     prepForm(Ejs *ejs, EjsHttp *hp, cchar *prefix, EjsObj *data);
static ssize    readHttpData(Ejs *ejs, EjsHttp *hp, ssize count);
static void     sendHttpCloseEvent(Ejs *ejs, EjsHttp *hp);
static void     sendHttpErrorEvent(Ejs *ejs, EjsHttp *hp);
static EjsHttp  *startHttpRequest(Ejs *ejs, EjsHttp *hp, char *method, int argc, EjsObj **argv);
static bool     waitForResponseHeaders(EjsHttp *hp);
static bool     waitForState(EjsHttp *hp, int state, MprTicks timeout, int throw);
static ssize    writeHttpData(Ejs *ejs, EjsHttp *hp);

/************************************ Methods *********************************/
/*  
    function Http(uri: Uri = null)
 */
static EjsHttp *httpConstructor(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ejsLoadHttpService(ejs);
    hp->ejs = ejs;

    if ((hp->conn = httpCreateConn(NULL, ejs->dispatcher)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    httpSetConnData(hp->conn, ejs);
    httpPrepClientConn(hp->conn, 0);
    httpSetConnNotifier(hp->conn, httpEventChange);
    httpSetConnContext(hp->conn, hp);
    if (argc == 1 && ejsIs(ejs, argv[0], Null)) {
        hp->uri = httpUriToString(((EjsUri*) argv[0])->uri, HTTP_COMPLETE_URI);
    }
    hp->method = sclone("GET");
    hp->requestContent = mprCreateBuf(ME_MAX_BUFFER, -1);
    hp->responseContent = mprCreateBuf(ME_MAX_BUFFER, -1);
    return hp;
}


/*  
    function get async(): Boolean
 */
static EjsBoolean *http_async(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return httpGetAsync(hp->conn) ? ESV(true) : ESV(false);
}


/*  
    function set async(enable: Boolean): Void
 */
static EjsObj *http_set_async(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    int         async;

    conn = hp->conn;
    async = (argv[0] == ESV(true));
    httpSetAsync(conn, async);
    httpSetIOCallback(conn, httpEvent);
    return 0;
}


#if ES_Http_available
/*  
    function get available(): Number
    DEPRECATED 1.0.0B3 (11/09)
 */
static EjsNumber *http_available(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprOff      len;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    len = httpGetContentLength(hp->conn);
    if (len > 0) {
        return ejsCreateNumber(ejs, (MprNumber) len);
    }
    /* Probably should be returning null here */
    return (EjsNumber*) ESV(minusOne);
}
#endif


/*
    function get ca(): String
 */
static EjsString *http_ca(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->caFile) {
        return ejsCreateStringFromAsc(ejs, hp->caFile);
    }
    return ESV(null);
}


/*  
    function set setCertificate(value: String): Void
 */
static EjsObj *http_set_ca(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->caFile = argv[0] == ESV(null) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    function close(): Void
 */
static EjsObj *http_close(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->conn) {
        if (hp->conn->state > HTTP_STATE_BEGIN) {
            httpFinalize(hp->conn);
        }
        sendHttpCloseEvent(ejs, hp);
        httpDestroyConn(hp->conn);
        hp->conn = httpCreateConn(NULL, ejs->dispatcher);
        httpPrepClientConn(hp->conn, 0);
        httpSetConnNotifier(hp->conn, httpEventChange);
        httpSetConnContext(hp->conn, hp);
    }
    return 0;
}


/*  
    function connect(method: String, url = null, data ...): Http
 */
static EjsHttp *http_connect(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->method = ejsToMulti(ejs, argv[0]);
    return startHttpRequest(ejs, hp, NULL, argc - 1, &argv[1]);
}


/*
    function get certificate(): String
 */
static EjsString *http_certificate(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->certFile) {
        return ejsCreateStringFromAsc(ejs, hp->certFile);
    }
    return ESV(null);
}


/*  
    function set setCertificate(value: String): Void
 */
static EjsObj *http_set_certificate(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->certFile = argv[0] == ESV(null) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    function get contentLength(): Number
 */
static EjsNumber *http_contentLength(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprOff      length;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    length = httpGetContentLength(hp->conn);
    return ejsCreateNumber(ejs, (MprNumber) length);
}


/*  
    function get contentType(): String
 */
static EjsString *http_contentType(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getStringHeader(ejs, hp, "CONTENT-TYPE");
}


/*  
    function get date(): Date
 */
static EjsDate *http_date(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getDateHeader(ejs, hp, "DATE");
}


/*  
    function finalize(): Void
 */
static EjsObj *http_finalize(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->conn) {
        httpFinalizeOutput(hp->conn);
        httpFlush(hp->conn);
    }
    return 0;
}


/*  
    function get finalized(): Boolean
 */
static EjsBoolean *http_finalized(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->conn) {
        return ejsCreateBoolean(ejs, hp->conn->tx->finalizedOutput);
    }
    return ESV(false);
}


/*  
    function flush(dir: Number = Stream.WRITE): Void
 */
static EjsObj *http_flush(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     dir;

    dir = (argc == 1) ? ejsGetInt(ejs, argv[0]) : EJS_STREAM_WRITE;
    if (dir & EJS_STREAM_WRITE) {
        httpFlush(hp->conn);
    }
    return 0;
}


/*  
    function form(uri: String = null, formData: Object = null): Http
    Issue a POST method with form data
 */
static EjsHttp *http_form(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsObj  *data;

    if (argc == 2 && !ejsIs(ejs, argv[1], Null)) {
        /*
            Prep here to reset the state. The ensures the current headers will be preserved.
            Users may have called setHeader to define custom headers. Users must call reset if they want to clear 
            prior headers.
         */
        httpPrepClientConn(hp->conn, 1);
        mprFlushBuf(hp->requestContent);
        data = argv[1];
        if (ejsGetLength(ejs, data) > 0) {
            prepForm(ejs, hp, NULL, data);
        } else {
            mprPutStringToBuf(hp->requestContent, ejsToMulti(ejs, data));
        }
        mprAddNullToBuf(hp->requestContent);
        httpSetHeader(hp->conn, "Content-Type", "application/x-www-form-urlencoded");
        /* Ensure this gets recomputed */
        httpRemoveHeader(hp->conn, "Content-Length");
    }
    return startHttpRequest(ejs, hp, "POST", argc, argv);
}


/*  
    function get followRedirects(): Boolean
 */
static EjsBoolean *http_followRedirects(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, hp->conn->followRedirects);
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
    function get(uri: String = null, ...data): Http
    The spec allows GET methods to have body data, but is rarely, if ever, used.
 */
static EjsHttp *http_get(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    startHttpRequest(ejs, hp, "GET", argc, argv);
    if (!ejs->exception && hp->conn) {
        httpFinalizeOutput(hp->conn);
        httpFlush(hp->conn);
    }
    return hp;
}


/*  
    Return the (proposed) request headers
    function getRequestHeaders(): Object
 */
static EjsPot *http_getRequestHeaders(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprKey          *kp;
    HttpConn        *conn;
    EjsPot          *headers;

    conn = hp->conn;
    headers = ejsCreateEmptyPot(ejs);
    for (kp = 0; conn->tx && (kp = mprGetNextKey(conn->tx->headers, kp)) != 0; ) {
        ejsSetPropertyByName(ejs, headers, EN(kp->key), ejsCreateStringFromAsc(ejs, kp->data));
    }
    return headers;
}


/*  
    function head(uri: String = null): Http
 */
static EjsHttp *http_head(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "HEAD", argc, argv);
}


/*  
    function header(key: String): String
 */
static EjsString *http_header(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsString   *result;
    cchar       *value;
    char        *str;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    str = slower(ejsToMulti(ejs, argv[0]));
    value = httpGetHeader(hp->conn, str);
    if (value) {
        result = ejsCreateStringFromAsc(ejs, value);
    } else {
        result = ESV(null);
    }
    return result;
}


/*  
    function get headers(): Object
 */
static EjsPot *http_headers(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprHash         *hash;
    MprKey          *kp;
    EjsPot          *results;
    int             i;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    results = ejsCreateEmptyPot(ejs);
    hash = httpGetHeaderHash(hp->conn);
    if (hash == 0) {
        return results;
    }
    for (i = 0, kp = mprGetFirstKey(hash); kp; kp = mprGetNextKey(hash, kp), i++) {
        ejsSetPropertyByName(ejs, results, EN(kp->key), ejsCreateStringFromAsc(ejs, kp->data));
    }
    return results;
}


/*
    function get info(): Object
 */
static EjsObj *http_info(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsObj  *obj;
    char    *key, *next, *value;

    if (hp->conn && hp->conn->sock) {
        obj = ejsCreateEmptyPot(ejs);
        for (key = stok(mprGetSocketState(hp->conn->sock), ",", &next); key; key = stok(NULL, ",", &next)) {
            ssplit(key, "=", &value);
            ejsSetPropertyByName(ejs, obj, EN(key), ejsCreateStringFromAsc(ejs, value));
        }
        return obj;
    }
    return ESV(null);
}


/*  
    function get isSecure(): Boolean
 */
static EjsBoolean *http_isSecure(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return ejsCreateBoolean(ejs, hp->conn->secure);
}


/*  
    function get key(): String
 */
static EjsAny *http_key(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->keyFile) {
        return ejsCreateStringFromAsc(ejs, hp->keyFile);
    }
    return ESV(null);
}


/*  
    function set key(keyFile: String): Void
 */
static EjsObj *http_set_key(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->keyFile = argv[0] == ESV(null) ? 0 : ejsToMulti(ejs, argv[0]);
    return 0;
}


/*  
    function get lastModified(): Date
 */
static EjsDate *http_lastModified(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return getDateHeader(ejs, hp, "LAST-MODIFIED");
}


/*
    function get limits(): Object
 */
static EjsObj *http_limits(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->limits == 0) {
        hp->limits = (EjsObj*) ejsCreateEmptyPot(ejs);
        ejsGetHttpLimits(ejs, hp->limits, hp->conn->limits, 0);
    }
    return hp->limits;
}


/*  
    function get method(): String
 */
static EjsString *http_method(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, hp->method);
}


/*  
    function set method(value: String): Void
 */
static EjsObj *http_set_method(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    cchar    *method;

    method = ejsToMulti(ejs, argv[0]);
    if (strcmp(method, "DELETE") != 0 && strcmp(method, "GET") != 0 &&  strcmp(method, "HEAD") != 0 &&
            strcmp(method, "OPTIONS") != 0 && strcmp(method, "POST") != 0 && strcmp(method, "PUT") != 0 &&
            strcmp(method, "TRACE") != 0) {
        ejsThrowArgError(ejs, "Unknown HTTP method");
        return 0;
    }
    hp->method = ejsToMulti(ejs, argv[0]);
    return 0;
}


/*
    function off(name, observer: function): Void
 */
static EjsObj *http_off(Ejs *ejs, EjsHttp *hp, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, hp->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function on(name, observer: function): Http
 */
static EjsHttp *http_on(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsFunction     *observer;
    HttpConn        *conn;

    observer = (EjsFunction*) argv[1];
    if (observer->boundThis == 0 || observer->boundThis == ejs->global) {
        observer->boundThis = hp;
    }
    ejsAddObserver(ejs, &hp->emitter, argv[0], observer);

    conn = hp->conn;
    if (conn->readq && conn->readq->count > 0) {
        ejsSendEvent(ejs, hp->emitter, "readable", NULL, hp);
    }
    //  TODO - don't need to test finalizedConnector
    if (!conn->tx->finalizedConnector && !conn->error && HTTP_STATE_CONNECTED <= conn->state && 
            conn->state < HTTP_STATE_FINALIZED && conn->writeq->ioCount == 0) {
        httpNotify(conn, HTTP_EVENT_WRITABLE, 0);
    }
    return hp;
}


/*
    function post(uri: String = null, ...requestContent): Http
 */
static EjsHttp *http_post(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "POST", argc, argv);
}


#if UNUSED
/*
    function get provider(): String
 */
static EjsString *http_provider(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    cchar   *name;

    if (hp->ssl && hp->ssl->provider) {
        name = hp->ssl->provider->name;
    } else {
        name = MPR->socketService->sslProvider;
    }
    return ejsCreateStringFromAsc(ejs, name);
}


/*
    function set provider(name: String): Void
 */
static EjsObj *http_set_provider(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (!hp->ssl) {
        hp->ssl = mprCreateSsl(0);
    }
    mprSetSslProvider(hp->ssl, ejsToMulti(ejs, argv[0]));
    return 0;
}


/*
    function get providers(): Array
 */
static EjsArray *http_providers(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsArray    *result;
    int         i;

    result = ejsCreateArray(ejs, 0);
    i = 0;
#if ME_COM_EST
    ejsSetProperty(ejs, result, i++, ejsCreateStringFromAsc(ejs, "est"));
#endif
#if ME_COM_OPENSSL
    ejsSetProperty(ejs, result, i++, ejsCreateStringFromAsc(ejs, "openssl"));
#endif
#if ME_COM_MATRIXSSL
    ejsSetProperty(ejs, result, i++, ejsCreateStringFromAsc(ejs, "matrixssl"));
#endif
#if ME_COM_MOCANA
    ejsSetProperty(ejs, result, i++, ejsCreateStringFromAsc(ejs, "mocana"));
#endif
    return result;
}
#endif


/*  
    function put(uri: String = null, form object): Http
 */
static EjsHttp *http_put(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return startHttpRequest(ejs, hp, "PUT", argc, argv);
}


/*  
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
    Returns a count of bytes read. Non-blocking if a callback is defined. Otherwise, blocks.

    Offset: -1 then read to the buffer write position, >= 0 then read to that offset
    count: -1 then read as much as the buffer will hold. If buffer is growable, read all content. If not growable, 
        read the buffer size. If count >= 0, then read that number of bytes.
 */
static EjsNumber *http_read(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsByteArray    *buffer;
    HttpConn        *conn;
    MprOff          contentLength;
    ssize           offset, count;

    conn = hp->conn;
    buffer = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]): -1;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    contentLength = httpGetContentLength(conn);
    if (conn->state >= HTTP_STATE_PARSED && contentLength == hp->readCount) {
        /* End of input */
        return ESV(null);
    }
    if (offset < 0) {
        offset = buffer->writePosition;
    } else if (offset < buffer->size) {
        ejsSetByteArrayPositions(ejs, buffer, offset, offset);
    } else {
        ejsThrowOutOfBoundsError(ejs, "Bad read offset value");
        return 0;
    }
    if (count < 0 && !buffer->resizable) {
        count = buffer->size - offset;
    }
    if ((count = readHttpData(ejs, hp, count)) < 0) {
        assert(ejs->exception);
        return 0;
    } else if (count == 0 && conn->state > HTTP_STATE_CONTENT) {
        return ESV(null);
    }
    hp->readCount += count;
    if (ejsCopyToByteArray(ejs, buffer, offset, (char*) mprGetBufStart(hp->responseContent), count) != count) {
        ejsThrowMemoryError(ejs);
    }
    ejsSetByteArrayPositions(ejs, buffer, -1, buffer->writePosition + count);
    mprAdjustBufStart(hp->responseContent, count);
    mprResetBufIfEmpty(hp->responseContent);
    return ejsCreateNumber(ejs, (MprNumber) count);
}


/*  
    function readString(count: Number = -1): String
    Read count bytes (default all) of content as a string. This always starts at the first character of content.
 */
static EjsString *http_readString(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsString   *result;
    ssize       count;
    
    count = (argc == 1) ? ejsGetInt(ejs, argv[0]) : -1;
    if (!waitForState(hp, HTTP_STATE_CONTENT, -1, 1)) {
        return 0;
    }
    if ((count = readHttpData(ejs, hp, count)) < 0) {
        assert(ejs->exception);
        return 0;
    } else if (count == 0 && hp->conn->state > HTTP_STATE_CONTENT) {
        return ESV(null);
    }
    //  UNICODE ENCODING
    result = ejsCreateStringFromMulti(ejs, mprGetBufStart(hp->responseContent), count);
    mprAdjustBufStart(hp->responseContent, count);
    mprResetBufIfEmpty(hp->responseContent);
    return result;
}


/*  
    function reset(): Void
 */
static EjsObj *http_reset(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpResetCredentials(hp->conn);
    httpPrepClientConn(hp->conn, 0);
    return 0;
}


/*  
    function get response(): String
 */
static EjsString *http_response(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    EjsAny  *response;

    if (hp->responseCache) {
        return hp->responseCache;
    }
    if ((response = http_readString(ejs, hp, argc, argv)) == ESV(null)) {
        return ESV(empty);
    }
    hp->responseCache = (EjsString*) response;
    return hp->responseCache;
}


/*  
    function set response(data: String): Void
 */
static EjsObj *http_set_response(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->responseCache = (EjsString*) argv[0];
    return 0;
}


/*  
    function get retries(): Number
 */
static EjsNumber *http_retries(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, hp->conn->retries);
}


/*  
    function set retries(count: Number): Void
 */
static EjsObj *http_set_retries(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    httpSetRetries(hp->conn, ejsGetInt(ejs, argv[0]));
    return 0;
}


/*  
    function setCredentials(username: String?, password: String?, authType: String?): Void
 */
static EjsObj *http_setCredentials(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    cchar   *authType, *password, *user;

    user = (argc <= 0) ? 0 : ejsToMulti(ejs, argv[0]);
    password = (argc <= 1) ? 0 : ejsToMulti(ejs, argv[1]);
    authType = (argc <= 2) ? 0 : ejsToMulti(ejs, argv[2]);
    if (ejsIs(ejs, argv[0], Null)) {
        httpResetCredentials(hp->conn);
    } else {
        httpSetCredentials(hp->conn, user, password, authType);
    }
    return 0;
}


/*  
    function setHeader(key: String, value: String, overwrite: Boolean = true): Void
 */
static EjsObj *http_setHeader(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    cchar       *key, *value;
    bool        overwrite;

    assert(argc >= 2);

    conn = hp->conn;
    if (conn->state >= HTTP_STATE_CONNECTED) {
        ejsThrowArgError(ejs, "Cannot update request headers once the request has started");
        return 0;
    }
    key = ejsToMulti(ejs, argv[0]);
    value = ejsToMulti(ejs, argv[1]);
    overwrite = (argc == 3) ? ejsGetBoolean(ejs, argv[2]) : 1;
    if (overwrite) {
        httpSetHeaderString(hp->conn, key, value);
    } else {
        httpAppendHeaderString(hp->conn, key, value);
    }
    return 0;
}


/*  
    function set limits(limits: Object): Void
 */
static EjsObj *http_setLimits(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (hp->conn->limits == ejs->http->clientLimits) {
        httpSetUniqueConnLimits(hp->conn);
    }
    if (hp->limits == 0) {
        hp->limits = (EjsObj*) ejsCreateEmptyPot(ejs);
        ejsGetHttpLimits(ejs, hp->limits, hp->conn->limits, 0);
    }
    ejsBlendObject(ejs, hp->limits, argv[0], EJS_BLEND_OVERWRITE);
    ejsSetHttpLimits(ejs, hp->conn->limits, hp->limits, 0);
    return 0;
}


static int getNumOption(Ejs *ejs, EjsObj *options, cchar *field)
{
    EjsObj      *obj;

    if ((obj = ejsGetPropertyByName(ejs, options, EN(field))) != 0) {
        return ejsGetInt(ejs, obj);
    }
    return -1;
}


static void setupTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options)
{
    httpSetTraceEventLevel(trace, "connection", getNumOption(ejs, options, "connection"));
    httpSetTraceEventLevel(trace, "error", getNumOption(ejs, options, "error"));
    httpSetTraceEventLevel(trace, "inform", getNumOption(ejs, options, "info"));
    httpSetTraceEventLevel(trace, "rxFirst", getNumOption(ejs, options, "rxFirst"));
    httpSetTraceEventLevel(trace, "rxHeaders", getNumOption(ejs, options, "rxHeaders"));
    httpSetTraceEventLevel(trace, "rxBody", getNumOption(ejs, options, "rxBody"));
    httpSetTraceEventLevel(trace, "txFirst", getNumOption(ejs, options, "txFirst"));
    httpSetTraceEventLevel(trace, "txHeaders", getNumOption(ejs, options, "txHeaders"));
    httpSetTraceEventLevel(trace, "txBody", getNumOption(ejs, options, "txBody"));
    httpSetTraceEventLevel(trace, "complete", getNumOption(ejs, options, "complete"));

    httpSetTraceContentSize(trace, getNumOption(ejs, options, "size"));
}


PUBLIC int ejsSetupHttpTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options)
{
    setupTrace(ejs, trace, options);
    return 0;
}


/*  
    function trace(options): Void
 */
static EjsObj *http_trace(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ejsSetupHttpTrace(ejs, hp->conn->trace, argv[0]);
    return 0;
}


/*  
    function get status(): Number
 */
static EjsNumber *http_status(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     code;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    code = httpGetStatus(hp->conn);
    if (code <= 0) {
        return ESV(null);
    }
    return ejsCreateNumber(ejs, code);
}


/*  
    function get statusMessage(): String
 */
static EjsString *http_statusMessage(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    HttpConn    *conn;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    conn = hp->conn;
    if (conn->errorMsg) {
        return ejsCreateStringFromAsc(ejs, conn->errorMsg);
    }
    return ejsCreateStringFromAsc(ejs, httpGetStatusMessage(hp->conn));
}


/*  
    function get uri(): Uri
 */
static EjsUri *http_uri(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    return ejsCreateUriFromAsc(ejs, hp->uri);
}


/*  
    function set uri(newUri: Uri): Void
 */
static EjsObj *http_set_uri(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    hp->uri = httpUriToString(((EjsUri*) argv[0])->uri, HTTP_COMPLETE_URI);
    return 0;
}


/*
    function get verify(): Boolean
 */
static EjsBoolean *http_verify(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (!hp->ssl) {
        hp->ssl = mprCreateSsl(0);
    }
    return hp->ssl->verifyPeer ?  ESV(true) : ESV(false);
}


/*  
    function set verify(on: Boolean): Void
 */
static EjsObj *http_set_verify(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     verify;

    verify = (argv[0] == ESV(true));
    if (!hp->ssl) {
        hp->ssl = mprCreateSsl(0);
    }
    mprVerifySslIssuer(hp->ssl, verify);
    mprVerifySslPeer(hp->ssl, verify);
    return 0;
}


/*
    function get verifyIssuer(): Boolean
 */
static EjsBoolean *http_verifyIssuer(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    if (!hp->ssl) {
        hp->ssl = mprCreateSsl(0);
    }
    return hp->ssl->verifyIssuer ?  ESV(true) : ESV(false);
}


/*  
    function set verifyIssuer(on: Boolean): Void
 */
static EjsObj *http_set_verifyIssuer(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    int     verifyIssuer;

    verifyIssuer = (argv[0] == ESV(true));
    if (!hp->ssl) {
        hp->ssl = mprCreateSsl(0);
    }
    mprVerifySslIssuer(hp->ssl, verifyIssuer);
    return 0;
}


/*  
    Wait for a request to complete. Timeout is in msec. Timeout < 0 means use default inactivity and request timeouts.
    Timeout of zero means no timeout.

    function wait(timeout: Number = -1): Boolean
 */
static EjsBoolean *http_wait(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    MprTicks    timeout;

    timeout = (argc >= 1) ? ejsGetInt(ejs, argv[0]) : -1;
    if (timeout == 0) {
        timeout = MPR_MAX_TIMEOUT;
    }
    if (!waitForState(hp, HTTP_STATE_FINALIZED, timeout, 0)) {
        return ESV(false);
    }
    return ESV(true);
}


/*
    function write(...data): Void
 */
static EjsNumber *http_write(Ejs *ejs, EjsHttp *hp, int argc, EjsObj **argv)
{
    ssize     nbytes;

    hp->data = ejsCreateByteArray(ejs, -1);
    if (ejsWriteToByteArray(ejs, hp->data, 1, &argv[0]) < 0) {
        return 0;
    }
    if ((nbytes = writeHttpData(ejs, hp)) < 0) {
        return 0;
    }
    hp->writeCount += nbytes;
    if (hp->conn->async) {
        if (ejsGetByteArrayAvailableData(hp->data) > 0) {
            httpEnableConnEvents(hp->conn);
        }
    }
    return ejsCreateNumber(ejs, (MprNumber) nbytes);
}


/*********************************** Support **********************************/
/*
    function [get|put|delete|post...](uri = null, ...data): Http
 */
static EjsHttp *startHttpRequest(Ejs *ejs, EjsHttp *hp, char *method, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *data;
    EjsNumber       *written;
    EjsUri          *uriObj;
    HttpConn        *conn;
    ssize           nbytes;

    conn = hp->conn;
    hp->responseCache = 0;
    hp->requestContentCount = 0;
    mprFlushBuf(hp->responseContent);

    if (argc >= 1 && !ejsIs(ejs, argv[0], Null)) {
        uriObj = (EjsUri*) argv[0];
        hp->uri = httpUriToString(uriObj->uri, HTTP_COMPLETE_URI);
    }
    if (argc == 2 && ejsIs(ejs, argv[1], Array)) {
        args = (EjsArray*) argv[1];
        if (args->length > 0) {
            data = ejsCreateByteArray(ejs, -1);
            written = ejsWriteToByteArray(ejs, data, 1, &argv[1]);
            mprPutBlockToBuf(hp->requestContent, (char*) data->value, (int) written->value);
            mprAddNullToBuf(hp->requestContent);
            assert(written > 0);
        }
    }
    if (hp->uri == 0) {
        ejsThrowArgError(ejs, "URL is not defined");
        return 0;
    }
    if (method && strcmp(hp->method, method) != 0) {
        hp->method = sclone(method);
    }
    if (hp->method == 0) {
        ejsThrowArgError(ejs, "HTTP Method is not defined");
        return 0;
    }
    if (hp->certFile) {
        if (!hp->ssl) {
            hp->ssl = mprCreateSsl(0);
        }
        mprSetSslCertFile(hp->ssl, hp->certFile);
        if (!hp->keyFile) {
            ejsThrowStateError(ejs, "Must define a Http.key to use with a certificate");
        }
        mprSetSslKeyFile(hp->ssl, hp->keyFile);
    }
    if (hp->caFile) {
        if (!hp->ssl) {
            hp->ssl = mprCreateSsl(0);
        }
        mprSetSslCaFile(hp->ssl, hp->caFile);
    }
    if (httpConnect(conn, hp->method, hp->uri, hp->ssl) < 0) {
        ejsThrowIOError(ejs, "Cannot issue request for \"%s\"", hp->uri);
        return 0;
    }
    if (mprGetBufLength(hp->requestContent) > 0) {
        nbytes = httpWriteBlock(conn->writeq, mprGetBufStart(hp->requestContent), mprGetBufLength(hp->requestContent), HTTP_BLOCK);
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Cannot write request data for \"%s\"", hp->uri);
            return 0;
        } else if (nbytes > 0) {
            assert(nbytes == mprGetBufLength(hp->requestContent));
            mprAdjustBufStart(hp->requestContent, nbytes);
            hp->requestContentCount += nbytes;
        }
        httpFinalizeOutput(conn);
        httpFlush(conn);
    }
    httpNotify(conn, HTTP_EVENT_WRITABLE, 0);
    if (conn->async) {
        httpEnableConnEvents(hp->conn);
    }
    return hp;
}


static void httpEventChange(HttpConn *conn, int event, int arg)
{
    Ejs         *ejs;
    EjsHttp     *hp;
    HttpTx      *tx;
    MprOff      lastWritten;

    hp = httpGetConnContext(conn);
    ejs = hp->ejs;
    tx = conn->tx;

    switch (event) {
    case HTTP_EVENT_STATE:
        switch (arg) {
        case HTTP_STATE_PARSED:
            if (hp->emitter) {
                ejsSendEvent(ejs, hp->emitter, "headers", NULL, hp);
            }
            break;

        case HTTP_STATE_FINALIZED:
            if (hp->emitter) {
                if (conn->error) {
                    sendHttpErrorEvent(ejs, hp);
                }
                sendHttpCloseEvent(ejs, hp);
            }
            break;
        }
        break;

    case HTTP_EVENT_READABLE:
        if (hp && hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "readable", NULL, hp);
        }
        break;

    case HTTP_EVENT_WRITABLE:
        if (hp && hp->emitter) {
            do {
                lastWritten = tx->bytesWritten;
                ejsSendEvent(ejs, hp->emitter, "writable", NULL, hp);
            } while (tx->bytesWritten > lastWritten && !tx->writeBlocked);
        }
        break;
    }
}


/*  
    Read the required number of bytes into the response content buffer. Count < 0 means transfer the entire content.
    Returns the number of bytes read. Returns null on EOF.
 */ 
static ssize readHttpData(Ejs *ejs, EjsHttp *hp, ssize count)
{
    MprBuf      *buf;
    HttpConn    *conn;
    ssize       len, space, nbytes;

    conn = hp->conn;
    buf = hp->responseContent;
    mprResetBufIfEmpty(buf);
    while (count < 0 || mprGetBufLength(buf) < count) {
        len = (count < 0) ? ME_MAX_BUFFER : (count - mprGetBufLength(buf));
        space = mprGetBufSpace(buf);
        if (space < len) {
            mprGrowBuf(buf, len - space);
        }
        if ((nbytes = httpRead(conn, mprGetBufEnd(buf), len)) < 0) {
            ejsThrowIOError(ejs, "Cannot read required data");
            return MPR_ERR_CANT_READ;
        }
        mprAdjustBufEnd(buf, nbytes);
        if (hp->conn->async || (nbytes == 0 && conn->state > HTTP_STATE_CONTENT)) {
            break;
        }
    }
    if (count < 0) {
        return mprGetBufLength(buf);
    }
    return min(count, mprGetBufLength(buf));
}


/* 
    Write another block of data
 */
static ssize writeHttpData(Ejs *ejs, EjsHttp *hp)
{
    EjsByteArray    *ba;
    HttpConn        *conn;
    ssize           count, nbytes;

    conn = hp->conn;
    ba = hp->data;
    nbytes = 0;
    if (ba && (count = ejsGetByteArrayAvailableData(ba)) > 0) {
        if (conn->tx->finalizedOutput) {
            ejsThrowIOError(ejs, "Cannot write to socket");
            return 0;
        }
        //  TODO - or should this be non-blocking
        nbytes = httpWriteBlock(conn->writeq, (cchar*) &ba->value[ba->readPosition], count, HTTP_BLOCK);
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Cannot write to socket");
            return 0;
        }
        ba->readPosition += nbytes;
    }
    httpServiceQueues(conn, HTTP_BLOCK);
    return nbytes;
}


/*  
    Respond to an IO event. This wraps the standard httpIOEvent() call.
 */
static void httpEvent(HttpConn *conn, MprEvent *event)
{
    EjsHttp     *hp;
    Ejs         *ejs;

    assert(conn->async);

    hp = conn->context;
    ejs = hp->ejs;

    httpIOEvent(conn, event);
    if (event->mask & MPR_WRITABLE) {
        if (hp->data) {
            writeHttpData(ejs, hp);
        }
    }
}


static EjsDate *getDateHeader(Ejs *ejs, EjsHttp *hp, cchar *key)
{
    MprTime     when;
    cchar       *value;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    value = httpGetHeader(hp->conn, key);
    if (value == 0) {
        return ESV(null);
    }
    if (mprParseTime(&when, value, MPR_UTC_TIMEZONE, NULL) < 0) {
        value = 0;
    }
    return ejsCreateDate(ejs, when);
}


static EjsString *getStringHeader(Ejs *ejs, EjsHttp *hp, cchar *key)
{
    cchar       *value;

    if (!waitForResponseHeaders(hp)) {
        return 0;
    }
    value = httpGetHeader(hp->conn, key);
    if (value == 0) {
        return ESV(null);
    }
    return ejsCreateStringFromAsc(ejs, value);
}


/*  
    Prepare form data as a series of key-value pairs. Data is formatted according to www-url-encoded specs by 
    mprSetHttpFormData. Objects are flattened into a one level key/value pairs. Keys can have embedded "." separators.
    E.g.  name=value&address=77%20Park%20Lane
 */
static void prepForm(Ejs *ejs, EjsHttp *hp, cchar *prefix, EjsObj *data)
{
    EjsName     qname;
    EjsObj      *vp;
    EjsString   *value;
    cchar       *key, *sep, *vstr;
    char        *encodedKey, *encodedValue, *newPrefix, *newKey;
    int         i, count;

    count = ejsGetLength(ejs, data);
    for (i = 0; i < count; i++) {
        if (ejsIs(ejs, data, Array)) {
            key = itos(i);
        } else {
            qname = ejsGetPropertyName(ejs, data, i);
            key = ejsToMulti(ejs, qname.name);
        }
        vp = ejsGetProperty(ejs, data, i);
        if (vp == 0) {
            continue;
        }
        if (ejsGetLength(ejs, vp) > 0) {
            if (prefix) {
                newPrefix = sfmt("%s.%s", prefix, key);
                prepForm(ejs, hp, newPrefix, vp);
            } else {
                prepForm(ejs, hp, key, vp);
            }
        } else {
            value = ejsToString(ejs, vp);
            sep = (mprGetBufLength(hp->requestContent) > 0) ? "&" : "";
            if (prefix) {
                newKey = sjoin(prefix, ".", key, NULL);
                encodedKey = mprUriEncode(newKey, MPR_ENCODE_URI_COMPONENT); 
            } else {
                encodedKey = mprUriEncode(key, MPR_ENCODE_URI_COMPONENT);
            }
            vstr = ejsToMulti(ejs, value);
            encodedValue = mprUriEncode(vstr, MPR_ENCODE_URI_COMPONENT);
            mprPutToBuf(hp->requestContent, "%s%s=%s", sep, encodedKey, encodedValue);
        }
    }
}

#if FUTURE && KEEP
/*  
    Prepare form data using json encoding. The objects are json encoded then URI encoded to be safe. 
 */
static void prepForm(Ejs *ejs, EjsHttp *hp, char *prefix, EjsObj *data)
{
    EjsName     qname;
    EjsObj      *vp;
    EjsString   *value;
    cchar       *key, *sep;
    char        *encodedKey, *encodedValue, *newPrefix, *newKey;
    int         i, count;

    jdata = ejsToJSON(ejs, data, NULL);
    if (prefix) {
        newKey = sjoin(prefix, ".", key, NULL);
        encodedKey = mprUriEncode(newKey, MPR_ENCODE_URI_COMPONENT); 
    } else {
        encodedKey = mprUriEncode(key, MPR_ENCODE_URI_COMPONENT);
    }
    encodedValue = mprUriEncode(value->value, MPR_ENCODE_URI_COMPONENT);
    mprPutToBuf(hp->requestContent, "%s%s=%s", sep, encodedKey, encodedValue);
}
#endif


#if FUTURE
static bool expired(EjsHttp *hp)
{
    int     requestTimeout, inactivityTimeout, diff, inactivity;

    requestTimeout = conn->limits->requestTimeout ? conn->limits->requestTimeout : MAXINT;
    inactivityTimeout = conn->limits->inactivityTimeout ? conn->limits->inactivityTimeout : MAXINT;

    /* 
        Workaround for a GCC bug when comparing two 64bit numerics directly. Need a temporary.
     */
    diff = (conn->lastActivity + inactivityTimeout) - http->now;
    inactivity = 1;
    if (diff > 0 && conn->rx) {
        diff = (conn->started + requestTimeout) - http->now;
        inactivity = 0;
    }
    if (diff < 0) {
        if (conn->rx) {
            if (inactivity) {
                mprDebug("ejs http", 4, "Inactive request timed out %s, exceeded inactivity timeout %d", 
                    conn->rx->uri, inactivityTimeout);
            } else {
                mprDebug("ejs http", 4, "Request timed out %s, exceeded timeout %d", conn->rx->uri, requestTimeout);
            }
        } else {
            mprDebug("ejs http", 4, "Idle connection timed out");
        }
    }
}
#endif


/*  
    Wait for the connection to acheive a requested state
    Timeout is in msec. Timeout of zero means don't wait. Timeout of < 0 means use standard inactivity and 
    duration timeouts.
 */
static bool waitForState(EjsHttp *hp, int state, MprTicks timeout, int throw)
{
    Ejs             *ejs;
    MprTicks        mark, remaining;
    HttpConn        *conn;
    HttpUri         *location, *uri;
    HttpRx          *rx;
    char            *url;
    int             count, redirectCount, success, rc;

    ejs = hp->ejs;
    conn = hp->conn;
    if (conn->state >= state) {
        return 1;
    }
    if (conn->state < HTTP_STATE_CONNECTED) {
        if (throw && ejs->exception == 0) {
            ejsThrowIOError(ejs, "Http request failed: not connected");
        }
        return 0;
    }
    if (!conn->async) {
        httpFinalizeOutput(conn);
    }
    httpFlush(conn);
    redirectCount = 0;
    success = count = 0;
    mark = mprGetTicks();
    remaining = timeout;
    while (conn->state < state && count <= conn->retries && redirectCount < 16 && !conn->error && 
            !ejs->exiting && !mprIsStopping(conn)) {
        count++;
        if ((rc = httpWait(conn, HTTP_STATE_PARSED, remaining)) == 0) {
            if (httpNeedRetry(conn, &url)) {
                if (url) {
                    httpRemoveHeader(conn, "Host");
                    if ((location = httpCreateUri(url, 0)) == 0) {
                        ejsThrowIOError(ejs, "Bad location Uri");
                        return 0;
                    }
                    uri = httpJoinUri(conn->tx->parsedUri, 1, &location);
                    hp->uri = httpUriToString(uri, HTTP_COMPLETE_URI);
                }
                count--; 
                redirectCount++;
            } else if (httpWait(conn, state, remaining) == 0) {
                success = 1;
                break;
            }
        } else {
            if (rc == MPR_ERR_CANT_CONNECT) {
                httpError(conn, HTTP_CODE_COMMS_ERROR, "Connection error");
            } else if (rc == MPR_ERR_TIMEOUT) {
                if (timeout > 0) {
                    httpError(conn, HTTP_CODE_REQUEST_TIMEOUT, "Request timed out");
                }
            } else if (rc == MPR_ERR_NOT_READY) {
                httpError(conn, HTTP_CODE_COMMS_ERROR, "Connection not ready / reset");
            } else if (rc == MPR_ERR_BAD_STATE) {
                httpError(conn, HTTP_CODE_COMMS_ERROR, "Connection in bad state");
            } else {
                httpError(conn, HTTP_CODE_NO_RESPONSE, "Client request error");
            }
            break;
        }
        rx = conn->rx;
        if (rx) {
            if (rx->status == HTTP_CODE_REQUEST_TOO_LARGE || rx->status == HTTP_CODE_REQUEST_URL_TOO_LARGE ||
                (rx->status == HTTP_CODE_UNAUTHORIZED && conn->username == 0)) {
                /* No point retrying */
                break;
            }
        }
        if (hp->writeCount > 0) {
            /* Cannot auto-retry with manual writes */
            break;
        }
        if (timeout > 0) {
            remaining = mprGetRemainingTicks(mark, timeout);
            if (count > 0 && remaining <= 0) {
                break;
            }
        }
        if (hp->requestContentCount > 0) {
            mprAdjustBufStart(hp->requestContent, -hp->requestContentCount);
        }
        /* Force a new connection */
        if (rx == 0 || rx->status != HTTP_CODE_UNAUTHORIZED) {
            httpSetKeepAliveCount(conn, -1);
        }
        httpPrepClientConn(conn, 1);
        startHttpRequest(ejs, hp, NULL, 0, NULL);
        if (ejs->exception) {
            return 0;
        }
        if (!conn->async) {
            httpFinalizeOutput(conn);
            httpFlush(conn);
        }
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
    Timeout < 0 means use default inactivity timeout. Timeout of zero means wait forever.
 */
static bool waitForResponseHeaders(EjsHttp *hp)
{
    if (hp->conn->state < HTTP_STATE_CONNECTED) {
        return 0;
    }
    if (hp->conn->state < HTTP_STATE_PARSED && !waitForState(hp, HTTP_STATE_PARSED, -1, 1)) {
        return 0;
    }
    return 1;
}


static EjsNumber *limitToNumber(Ejs *ejs, uint64 n)
{
    if (n == HTTP_UNLIMITED || n > EJS_MAX_INT) {
        return ESV(infinity);
    } else {
        return ejsCreateNumber(ejs, (MprNumber) n);
    }
}


/*
    Get limits:  obj[*] = limits
 */
PUBLIC void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, bool server) 
{
    EjsNumber   *n;

    ejsSetPropertyByName(ejs, obj, EN("chunk"), limitToNumber(ejs, limits->chunkSize));
    ejsSetPropertyByName(ejs, obj, EN("connReuse"), limitToNumber(ejs, limits->keepAliveMax));
    ejsSetPropertyByName(ejs, obj, EN("receive"), limitToNumber(ejs, limits->rxBodySize));
    ejsSetPropertyByName(ejs, obj, EN("transmission"), limitToNumber(ejs, limits->txBodySize));
    ejsSetPropertyByName(ejs, obj, EN("upload"), limitToNumber(ejs, limits->uploadSize));

    n = limitToNumber(ejs, limits->inactivityTimeout);
    if (n->value != ((EjsNumber*) ESV(infinity))->value) {
        n->value /= 1000;
    }
    ejsSetPropertyByName(ejs, obj, EN("inactivityTimeout"), n);

    n = limitToNumber(ejs, limits->requestTimeout);
    if (n->value != ((EjsNumber*) ESV(infinity))->value) {
        n->value /= 1000;
    }
    ejsSetPropertyByName(ejs, obj, EN("requestTimeout"), n);

    n = limitToNumber(ejs, limits->sessionTimeout);
    if (n->value != ((EjsNumber*) ESV(infinity))->value) {
        n->value /= 1000;
    }
    ejsSetPropertyByName(ejs, obj, EN("sessionTimeout"), n);

    if (server) {
        ejsSetPropertyByName(ejs, obj, EN("clients"), limitToNumber(ejs, limits->clientMax));
        ejsSetPropertyByName(ejs, obj, EN("connections"), limitToNumber(ejs, limits->connectionsMax));
        ejsSetPropertyByName(ejs, obj, EN("header"), limitToNumber(ejs, limits->headerSize));
        ejsSetPropertyByName(ejs, obj, EN("headers"), limitToNumber(ejs, limits->headerMax));
        ejsSetPropertyByName(ejs, obj, EN("requests"), limitToNumber(ejs, limits->requestsPerClientMax));
        ejsSetPropertyByName(ejs, obj, EN("stageBuffer"), limitToNumber(ejs, limits->bufferSize));
        ejsSetPropertyByName(ejs, obj, EN("uri"), limitToNumber(ejs, limits->uriSize));
    }
}


static uint64 numberToLimit(Ejs *ejs, EjsObj *obj, cchar *field)
{
    EjsObj      *vp;
    EjsNumber   *n;

    if ((vp = ejsGetPropertyByName(ejs, obj, EN(field))) != 0) {
        n = ejsToNumber(ejs, vp);
        if (n->value == ((EjsNumber*) ESV(infinity))->value || n->value >= EJS_MAX_INT) {
            return HTTP_UNLIMITED;
        } else {
            return ejsGetInt64(ejs, n);
        }
    }
    return 0;
}


PUBLIC void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, bool server) 
{
    /*
        TODO - the Limit object should map the Http limits more closely
     */
    limits->chunkSize =  (ssize) numberToLimit(ejs, obj, "chunk");
    limits->rxBodySize = (MprOff) numberToLimit(ejs, obj, "receive");
    limits->rxFormSize = (MprOff) numberToLimit(ejs, obj, "receive");
    limits->txBodySize = (MprOff) numberToLimit(ejs, obj, "transmission");
    limits->uploadSize = (MprOff) numberToLimit(ejs, obj, "upload");
    limits->keepAliveMax = (int) numberToLimit(ejs, obj, "connReuse");

    limits->inactivityTimeout = (MprTicks) numberToLimit(ejs, obj, "inactivityTimeout");
    if (limits->inactivityTimeout != HTTP_UNLIMITED) {
        limits->inactivityTimeout *= 1000;
    }
    limits->requestTimeout = (MprTicks) numberToLimit(ejs, obj, "requestTimeout");
    if (limits->requestTimeout != HTTP_UNLIMITED) {
        limits->requestTimeout *= 1000;
    }
    limits->sessionTimeout = (MprTicks) numberToLimit(ejs, obj, "sessionTimeout");
    if (limits->sessionTimeout != HTTP_UNLIMITED) {
        limits->sessionTimeout *= 1000;
    }
    if (limits->requestTimeout <= 0) {
        limits->requestTimeout = HTTP_UNLIMITED;
    }
    if (limits->inactivityTimeout <= 0) {
        limits->inactivityTimeout = HTTP_UNLIMITED;
    }
    if (limits->sessionTimeout <= 0) {
        limits->sessionTimeout = HTTP_UNLIMITED;
    }
    if (server) {
        limits->bufferSize = (ssize) numberToLimit(ejs, obj, "stageBuffer");
        limits->clientMax = (int) numberToLimit(ejs, obj, "clients");
        limits->connectionsMax = (int) numberToLimit(ejs, obj, "connections");
        limits->requestsPerClientMax = (int) numberToLimit(ejs, obj, "requests");
        limits->uriSize = (ssize) numberToLimit(ejs, obj, "uri");
        limits->headerMax = (int) numberToLimit(ejs, obj, "headers");
        limits->headerSize = (ssize) numberToLimit(ejs, obj, "header");
    }
}


static void sendHttpCloseEvent(Ejs *ejs, EjsHttp *hp)
{
    if (!hp->closed && ejs->service) {
        hp->closed = 1;
        if (hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "close", NULL, hp);
        }
    }
}


static void sendHttpErrorEvent(Ejs *ejs, EjsHttp *hp)
{
    if (!hp->error) {
        hp->error = 1;
        if (hp->emitter) {
            ejsSendEvent(ejs, hp->emitter, "error", NULL, hp);
        }
    }
}


/*********************************** Factory **********************************/

/*  
    Manage the object properties for the garbage collector
 */
static void manageHttp(EjsHttp *hp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(hp->emitter);
        mprMark(hp->data);
        mprMark(hp->limits);
        mprMark(hp->responseCache);
        mprMark(hp->conn);
        mprMark(hp->ssl);
        mprMark(hp->requestContent);
        mprMark(hp->responseContent);
        mprMark(hp->uri);
        mprMark(hp->method);
        mprMark(hp->caFile);
        mprMark(hp->certFile);
        mprMark(TYPE(hp));

    } else if (flags & MPR_MANAGE_FREE) {
        if (hp->conn && !hp->conn->destroyed) {
            sendHttpCloseEvent(hp->ejs, hp);
            httpDestroyConn(hp->conn);
            hp->conn = 0;
        }
    }
}


PUBLIC void ejsConfigureHttpType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Http"), sizeof(EjsHttp), manageHttp, EJS_TYPE_OBJ)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, httpConstructor);
#if UNUSED
#if ES_Http_providers
    ejsBindMethod(ejs, type, ES_Http_providers, http_providers);
#endif
#endif

    ejsBindAccess(ejs, prototype, ES_Http_async, http_async, http_set_async);
#if ES_Http_available
    /* DEPRECATED */
    ejsBindMethod(ejs, prototype, ES_Http_available, http_available);
#endif
    ejsBindAccess(ejs, prototype, ES_Http_ca, http_ca, http_set_ca);
    ejsBindMethod(ejs, prototype, ES_Http_close, http_close);
    ejsBindMethod(ejs, prototype, ES_Http_connect, http_connect);
    ejsBindAccess(ejs, prototype, ES_Http_certificate, http_certificate, http_set_certificate);
    ejsBindMethod(ejs, prototype, ES_Http_contentLength, http_contentLength);
    ejsBindMethod(ejs, prototype, ES_Http_contentType, http_contentType);
    ejsBindMethod(ejs, prototype, ES_Http_date, http_date);
    ejsBindMethod(ejs, prototype, ES_Http_finalize, http_finalize);
    ejsBindMethod(ejs, prototype, ES_Http_finalized, http_finalized);
    ejsBindMethod(ejs, prototype, ES_Http_flush, http_flush);
    ejsBindAccess(ejs, prototype, ES_Http_followRedirects, http_followRedirects, http_set_followRedirects);
    ejsBindMethod(ejs, prototype, ES_Http_form, http_form);
    ejsBindMethod(ejs, prototype, ES_Http_get, http_get);
    ejsBindMethod(ejs, prototype, ES_Http_getRequestHeaders, http_getRequestHeaders);
    ejsBindMethod(ejs, prototype, ES_Http_head, http_head);
    ejsBindMethod(ejs, prototype, ES_Http_header, http_header);
    ejsBindMethod(ejs, prototype, ES_Http_headers, http_headers);
    ejsBindMethod(ejs, prototype, ES_Http_isSecure, http_isSecure);
    ejsBindAccess(ejs, prototype, ES_Http_key, http_key, http_set_key);
    ejsBindMethod(ejs, prototype, ES_Http_lastModified, http_lastModified);
    ejsBindMethod(ejs, prototype, ES_Http_limits, http_limits);
    ejsBindAccess(ejs, prototype, ES_Http_method, http_method, http_set_method);
    ejsBindMethod(ejs, prototype, ES_Http_off, http_off);
    ejsBindMethod(ejs, prototype, ES_Http_on, http_on);
    ejsBindMethod(ejs, prototype, ES_Http_post, http_post);
#if UNUSED
    ejsBindAccess(ejs, prototype, ES_Http_provider, http_provider, http_set_provider);
#endif
    ejsBindMethod(ejs, prototype, ES_Http_put, http_put);
    ejsBindMethod(ejs, prototype, ES_Http_read, http_read);
    ejsBindMethod(ejs, prototype, ES_Http_readString, http_readString);
    ejsBindMethod(ejs, prototype, ES_Http_reset, http_reset);
    ejsBindAccess(ejs, prototype, ES_Http_response, http_response, http_set_response);
    ejsBindAccess(ejs, prototype, ES_Http_retries, http_retries, http_set_retries);
    ejsBindMethod(ejs, prototype, ES_Http_setCredentials, http_setCredentials);
    ejsBindMethod(ejs, prototype, ES_Http_setHeader, http_setHeader);
    ejsBindMethod(ejs, prototype, ES_Http_setLimits, http_setLimits);
    ejsBindAccess(ejs, prototype, ES_Http_info, http_info, NULL);
    ejsBindMethod(ejs, prototype, ES_Http_status, http_status);
    ejsBindMethod(ejs, prototype, ES_Http_statusMessage, http_statusMessage);
    ejsBindMethod(ejs, prototype, ES_Http_trace, http_trace);
    ejsBindAccess(ejs, prototype, ES_Http_uri, http_uri, http_set_uri);
    ejsBindAccess(ejs, prototype, ES_Http_verify, http_verify, http_set_verify);
    ejsBindAccess(ejs, prototype, ES_Http_verifyIssuer, http_verifyIssuer, http_set_verifyIssuer);
    ejsBindMethod(ejs, prototype, ES_Http_write, http_write);
    ejsBindMethod(ejs, prototype, ES_Http_wait, http_wait);
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
