/**
    ejsWebSocket.c - WebSocket class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

#if BIT_HTTP_WEB_SOCKETS
/*********************************** Forwards *********************************/

static void onWebSocketEvent(EjsWebSocket *ws, int event, EjsAny *data, HttpPacket *packet);
static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws);
static bool waitForHttpState(EjsWebSocket *ws, int state, MprTicks timeout, int throw);
static bool waitForReadyState(EjsWebSocket *ws, int state, MprTicks timeout, int throw);
static void webSocketNotify(HttpConn *conn, int state, int notifyFlags);

/************************************ Methods *********************************/
/*  
    function WebSocket(uri: Uri, protocols = null, options)

    options = {
        certificate: Path,
        verify: Boolean,
    }
 */
static EjsWebSocket *wsConstructor(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    EjsAny      *certificate;
    bool        verify;

    assert(ejsIsPot(ejs, ws));

    ejsLoadHttpService(ejs);
    ws->ejs = ejs;
    verify = 0;

    ws->uri = httpUriToString(((EjsUri*) argv[0])->uri, 0);
    if (argc >= 2) {
        if (ejsIs(ejs, argv[1], Array)) {
            ws->protocols = sclone((ejsToString(ejs, argv[1]))->value);
        } else if (ejsIs(ejs, argv[1], String)) {
            ws->protocols = sclone(((EjsString*) argv[1])->value);
        } else {
            ws->protocols = sclone("chat");
        }
    } else {
        ws->protocols = sclone("chat");
    }
    if (*ws->protocols == '\0') {
        ejsThrowArgError(ejs, "Bad protocol");
        return 0;
    }
    if (argc >= 3) {
        ws->frames = ejsGetPropertyByName(ejs, argv[2], EN("frames")) == ESV(true);
        verify = ejsGetPropertyByName(ejs, argv[2], EN("verify")) == ESV(true);
        if ((certificate = ejsGetPropertyByName(ejs, argv[2], EN("certificate"))) != 0) {
            ws->certFile = ejsToMulti(ejs, argv[0]);
        }
    }
    if ((ws->conn = httpCreateConn(MPR->httpService, NULL, ejs->dispatcher)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    httpSetAsync(ws->conn, 1);
    httpPrepClientConn(ws->conn, 0);
    httpSetConnNotifier(ws->conn, webSocketNotify);
    httpSetWebSocketProtocols(ws->conn, ws->protocols);
    httpSetConnContext(ws->conn, ws);
    if (sstarts(ws->uri, "wss")) {
        ws->ssl = mprCreateSsl(0);
        mprVerifySslIssuer(ws->ssl, verify);
        mprVerifySslPeer(ws->ssl, verify);
#if FUTURE
        if (!hp->caFile) {
            //MOB - Some define for this.
            hp->caFile = mprJoinPath(mprGetAppDir(), "http-ca.crt");
        }
        mprSetSslCaFile(hp->ssl, hp->caFile);
        mprSetSslCaFile(hp->ssl, mprJoinPath(mprGetAppDir(), "http-ca.crt"));
#endif
    }
    startWebSocketRequest(ejs, ws);
    return ws;
}


/*
    function get binaryType(): String
    NOTE: always returns ByteArray
 */
static EjsString *ws_binaryType(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, "ByteArray");
}


/*  
    function get bufferedAmount(): Number

    Returns amount of buffered send data
 */
static EjsNumber *ws_bufferedAmount(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, (MprNumber) ws->conn->writeq->count);
}


/*  
    function close(status: Number = 1000, reason: String? = ""): Void
 */
static EjsObj *ws_close(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    char        *reason;
    int         status;

    conn = ws->conn;
    if (conn) {
        status = argc == 0 ? WS_STATUS_OK : ejsGetInt(ejs, argv[0]);
        if (status <= 999 || status >= WS_STATUS_MAX || status == WS_STATUS_NO_STATUS || status == WS_STATUS_COMMS_ERROR) {
            ejsThrowArgError(ejs, "Bad status");
            return 0;
        }
        reason = (argc >= 1) ? ejsToMulti(ejs, argv[1]): 0; 
        if (slen(reason) >= 124) {
            ejsThrowArgError(ejs, "Close reason is too long. Must be less than 124 bytes");
            return 0;
        }
        httpSendClose(conn, status, reason);
    }
    return 0;
}


/*  
    function get extensions(): String
 */
static EjsString *ws_extensions(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ESV(empty);
}


/*
    function off(name, observer: function): Void
 */
static EjsObj *ws_off(Ejs *ejs, EjsWebSocket *ws, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, ws->emitter, argv[0], argv[1]);
    return 0;
}


/*  
    function on(name, observer: function): Http
 */
static EjsWebSocket *ws_on(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    EjsFunction     *observer;
    HttpConn        *conn;

    observer = (EjsFunction*) argv[1];
    if (observer->boundThis == 0 || observer->boundThis == ejs->global) {
        observer->boundThis = ws;
    }
    ejsAddObserver(ejs, &ws->emitter, argv[0], observer);

    conn = ws->conn;
    if (conn->readq && conn->readq->count > 0) {
        //  MOB - can't have NULL as data
        onWebSocketEvent(ws, HTTP_EVENT_READABLE, 0, 0);
    }
    //  MOB - don't need to test finalizedConnector
    if (!conn->tx->finalizedConnector && 
            !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_FINALIZED &&
            conn->writeq->ioCount == 0) {
        onWebSocketEvent(ws, HTTP_EVENT_WRITABLE, 0, 0);
    }
    return ws;
}


/*  
    function get protocol(): String
 */
static EjsString *ws_protocol(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, ws->protocols);
}


/*  
    function get readyState(): Number
 */
static EjsNumber *ws_readyState(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    //  MOB - should have API for this
    return ejsCreateNumber(ejs, (MprNumber) ws->conn->rx->webSocket->state);
}


/*  
    function send(...content): Number
 */
static EjsNumber *ws_send(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *ba;
    EjsAny          *arg;
    ssize           nbytes;
    int             i;

    args = (EjsArray*) argv[0];
    if (ws->conn->state < HTTP_STATE_PARSED && !waitForHttpState(ws, HTTP_STATE_PARSED, -1, 1)) {
        return ESV(null);
    }
    nbytes = 0;
    for (i = 0; i < args->length; i++) {
        if ((arg = ejsGetProperty(ejs, args, i)) != 0) {
            if (ejsIs(ejs, arg, ByteArray)) {
                ba = (EjsByteArray*) arg;
                nbytes = ejsGetByteArrayAvailableData(ba);
                nbytes = httpSendBlock(ws->conn, WS_MSG_BINARY, (cchar*) &ba->value[ba->readPosition], nbytes, HTTP_BLOCK);
            } else {
                nbytes = httpSend(ws->conn, ejsToMulti(ejs, arg));
            }
            if (nbytes < 0) {
                return ESV(null);
            }
        }
    }
    return ejsCreateNumber(ejs, (MprNumber) nbytes);
}


/*  
    function sendBlock(content, options): Number
 */
static EjsNumber *ws_sendBlock(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    EjsByteArray    *ba;
    EjsAny          *content, *vp;
    ssize           nbytes;
    cchar           *str;
    int             last, mode, type, flags;

    assert(argc == 2);

    if (ws->conn->state < HTTP_STATE_PARSED && !waitForHttpState(ws, HTTP_STATE_PARSED, -1, 1)) {
        return ESV(null);
    }
    content = argv[0];
    last = ejsGetPropertyByName(ejs, argv[1], EN("last")) != ESV(false);
    if ((vp = ejsGetPropertyByName(ejs, argv[1], EN("mode"))) != 0) {
        mode = (int) ejsGetNumber(ejs, vp);
        if (mode != HTTP_BUFFER && mode != HTTP_BLOCK && mode != HTTP_NON_BLOCK) {
            ejsThrowArgError(ejs, "Bad message mode");
            return 0;
        }
    } else {
        mode = HTTP_BUFFER;
    }
    if ((vp = ejsGetPropertyByName(ejs, argv[1], EN("type"))) != 0) {
        type = (int) ejsGetNumber(ejs, vp);
        if (type != WS_MSG_CONT && type != WS_MSG_TEXT && type != WS_MSG_BINARY) {
            ejsThrowArgError(ejs, "Bad message type");
            return 0;
        }
    } else {
        type = WS_MSG_TEXT;
    }
    flags = mode;
    if (!last) {
        flags |= HTTP_MORE;
    }
    if (ejsIs(ejs, content, ByteArray)) {
        ba = (EjsByteArray*) content;
        nbytes = ejsGetByteArrayAvailableData(ba);
        nbytes = httpSendBlock(ws->conn, type, (cchar*) &ba->value[ba->readPosition], nbytes, flags);
    } else {
        str = ejsToMulti(ejs, content);
        nbytes = httpSendBlock(ws->conn, type, str, slen(str), flags);
    }
    if (nbytes < 0) {
        ejsThrowIOError(ejs, "Cannot send block");
        return 0;
    }
    return ejsCreateNumber(ejs, (MprNumber) nbytes);
}


/*
    function get url(): Uri
 */
static EjsUri *ws_url(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateUriFromAsc(ejs, ws->uri);
}


/*
    Wait for a request to complete. Timeout is in msec. Timeout < 0 means use default inactivity and request timeouts.
    Timeout of zero means no timeout.

    function wait(state: Number, timeout: Number = -1): Boolean
 */
static EjsUri *ws_wait(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    MprTicks    timeout;
    int         state;

    state = argc >= 1 ? ejsGetInt(ejs, argv[0]) : WS_STATE_CLOSED;
    timeout = argc >= 2 ? ejsGetInt(ejs, argv[1]) : -1;
    if (timeout == 0) {
        timeout = MPR_MAX_TIMEOUT;
    }
    if (!waitForReadyState(ws, state, timeout, 0)) {
        return ESV(false);
    }
    return ESV(true);
}

/*********************************** Support **********************************/

static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws)
{
    HttpConn        *conn;

    conn = ws->conn;
    if (ws->certFile) {
        if (!ws->ssl) {
            ws->ssl = mprCreateSsl(0);
        }
        mprSetSslCertFile(ws->ssl, ws->certFile);
    }
    if (httpConnect(conn, "GET", ws->uri, ws->ssl) < 0) {
        ejsThrowIOError(ejs, "Cannot issue request for \"%s\"", ws->uri);
        return 0;
    }
    httpEnableConnEvents(ws->conn);
    return 0;
}


static void onWebSocketEvent(EjsWebSocket *ws, int event, EjsAny *data, HttpPacket *packet)
{
    Ejs             *ejs;
    EjsAny          *eobj;
    EjsFunction     *fn;
    HttpRx          *rx;
    cchar           *eventName, *reason;
    int             slot, status;

    ejs = ws->ejs;
    rx = ws->conn->rx;
    eobj = ejsCreateObj(ejs, ESV(Object), 0);
    slot = -1;
    eventName = 0;

    switch(event) {
    case HTTP_EVENT_READABLE:
        slot = ES_WebSocket_onmessage;
        eventName = "readable";
        assert(data);
        ejsSetPropertyByName(ejs, eobj, EN("data"), data);
        ejsSetPropertyByName(ejs, eobj, EN("last"), ejsCreateBoolean(ejs, packet->last));
        ejsSetPropertyByName(ejs, eobj, EN("type"), ejsCreateNumber(ejs, packet->type));
        break;

    case HTTP_EVENT_ERROR:
        eventName = "error";
        slot = ES_WebSocket_onerror;
        break;

    case HTTP_EVENT_APP_OPEN:
        slot = ES_WebSocket_onopen;
        eventName = "headers";
        if (rx->webSocket) {
            httpSetWebSocketPreserveFrames(ws->conn, ws->frames);
        }
        break;

    case HTTP_EVENT_DESTROY:
        if (ws->closed) {
            break;
        }
        ws->closed = 1;
        /* Fall through to close */

    case HTTP_EVENT_APP_CLOSE:
        eventName = "complete";
        slot = ES_WebSocket_onclose;
        status = rx ? rx->webSocket->closeStatus: WS_STATUS_COMMS_ERROR;
        reason = rx ? rx->webSocket->closeReason: 0;
        ejsSetPropertyByName(ejs, eobj, EN("code"), ejsCreateNumber(ejs, status));
        ejsSetPropertyByName(ejs, eobj, EN("reason"), ejsCreateStringFromAsc(ejs, reason));
        ejsSetPropertyByName(ejs, eobj, EN("wasClean"), ejsCreateBoolean(ejs, status != WS_STATUS_COMMS_ERROR));
        break;
    }
    if (slot >= 0) {
        if (ws->emitter) {
            ejsSendEvent(ejs, ws->emitter, eventName, ws, data);
        }
        fn = ejsGetProperty(ejs, ws, slot);
        if (ejsIsFunction(ejs, fn) && !ejs->exception) {
            ejsRunFunction(ejs, fn, ws, 1, &eobj);
        }
    }
}


/*
    Connection callback
 */
static void webSocketNotify(HttpConn *conn, int event, int arg)
{
    Ejs             *ejs;
    EjsWebSocket    *ws;
    EjsByteArray    *ba;
    EjsAny          *data;
    HttpPacket      *packet;
    MprBuf          *content;
    ssize           len;

    if ((ws = httpGetConnContext(conn)) == 0) {
        return;
    }
    ejs = ws->ejs;
    if (!ejs->service) {
        /* Shutting down */
        return;
    }
    switch (event) {
    case HTTP_EVENT_STATE:
        if (arg == HTTP_STATE_CONTENT) {
            ws->protocol = (char*) httpGetHeader(conn, "Sec-WebSocket-Protocol");
            mprTrace(3, "Web socket protocol %s", ws->protocol);
            onWebSocketEvent(ws, HTTP_EVENT_APP_OPEN, 0, 0);
        }
        break;
    
    case HTTP_EVENT_READABLE:
        packet = httpGetPacket(conn->readq);
        content = packet->content;
        if (packet->type == WS_MSG_TEXT) {
            data = ejsCreateStringFromBytes(ejs, mprGetBufStart(content), mprGetBufLength(content));
        } else {
            len = httpGetPacketLength(packet);
            assert(len > 0);
            if ((ba = ejsCreateByteArray(ejs, len)) == 0) {
                return;
            }
            memcpy(ba->value, mprGetBufStart(content), len);
            ejsSetByteArrayPositions(ejs, ba, -1, len);
            data = ba;
        }
        onWebSocketEvent(ws, event, data, packet);
        break;

    case HTTP_EVENT_ERROR:
        if (!ws->error && !ws->closed) {
            ws->error = 1;
            onWebSocketEvent(ws, event, 0, 0);
            ws->closed = 1;
            onWebSocketEvent(ws, HTTP_EVENT_APP_CLOSE, 0, 0);
        }
        break;

    case HTTP_EVENT_APP_CLOSE:
        if (!ws->closed) {
            ws->closed = 1;
            onWebSocketEvent(ws, event, 0, 0);
        }
        break;
    }
}


static bool waitForHttpState(EjsWebSocket *ws, int state, MprTicks timeout, int throw)
{
    Ejs             *ejs;
    MprTicks        mark, remaining;
    HttpConn        *conn;
    HttpUri         *uri;
    char            *url;
    int             count, redirectCount, success, rc;

    assert(state >= HTTP_STATE_PARSED);

    ejs = ws->ejs;
    conn = ws->conn;
    assert(conn->state >= HTTP_STATE_CONNECTED);

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
        httpFinalize(conn);
    }
    redirectCount = 0;
    success = count = 0;
    mark = mprGetTicks();
    remaining = timeout;
    while (conn->state < state && count <= conn->retries && redirectCount < 16 && 
           !conn->error && !ejs->exiting && !mprIsStopping(conn)) {
        count++;
        if ((rc = httpWait(conn, HTTP_STATE_PARSED, remaining)) == 0) {
            if (httpNeedRetry(conn, &url)) {
                if (url) {
                    uri = httpCreateUri(url, 0);
                    httpCompleteUri(uri, httpCreateUri(ws->uri, 0));
                    ws->uri = httpUriToString(uri, HTTP_COMPLETE_URI);
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
            } else {
                httpError(conn, HTTP_CODE_NO_RESPONSE, "Client request error");
            }
            break;
        }
        if (conn->rx) {
            if (conn->rx->status == HTTP_CODE_REQUEST_TOO_LARGE || conn->rx->status == HTTP_CODE_REQUEST_URL_TOO_LARGE) {
                /* No point retrying */
                break;
            }
        }
        if (timeout > 0) {
            remaining = mprGetRemainingTicks(mark, timeout);
            if (count > 0 && remaining <= 0) {
                break;
            }
        }
        /* Force a new connection */
        if (conn->rx == 0 || conn->rx->status != HTTP_CODE_UNAUTHORIZED) {
            httpSetKeepAliveCount(conn, -1);
        }
        httpPrepClientConn(conn, 1);
        if (startWebSocketRequest(ejs, ws) < 0) {
            return 0;
        }
        if (!conn->async) {
            httpFinalize(conn);
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


static bool waitForReadyState(EjsWebSocket *ws, int state, MprTicks timeout, int throw)
{
    Ejs             *ejs;
    HttpConn        *conn;
    HttpRx          *rx;
    MprTicks        mark, remaining, inactivityTimeout;
    int             eventMask;

    ejs = ws->ejs;
    conn = ws->conn;
    rx = conn->rx;
    assert(conn->state >= HTTP_STATE_CONNECTED);

    if (!rx || !rx->webSocket) {
        return 0;
    }
    eventMask = MPR_READABLE;
    if (!conn->tx->finalizedConnector) {
        eventMask |= MPR_WRITABLE;
    }
    if (rx->webSocket->state < state) {
        httpSetupWaitHandler(conn, eventMask);
    }
    if (mprGetDebugMode()) {
        inactivityTimeout = timeout = MPR_MAX_TIMEOUT;
    } else {
        inactivityTimeout = timeout < 0 ? conn->limits->inactivityTimeout : MPR_MAX_TIMEOUT;
    }
    if (timeout < 0) {
        timeout = conn->limits->requestTimeout;
    }
    mark = mprGetTicks();
    remaining = timeout;
    while (conn->state < HTTP_STATE_CONTENT || rx->webSocket->state < state) {
        if (conn->error || ejs->exiting || mprIsStopping(conn) || remaining < 0) {
            break;
        }
        mprWaitForEvent(conn->dispatcher, min(inactivityTimeout, remaining));
        remaining = mprGetRemainingTicks(mark, timeout);
    }
    return rx->webSocket->state >= state;
}


/*********************************** Factory **********************************/

/*  
    Manage the object properties for the garbage collector
 */
static void manageWebSocket(EjsWebSocket *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->conn);
        mprMark(ws->emitter);
        mprMark(ws->ssl);
        mprMark(ws->certFile);
        mprMark(ws->protocols);
        mprMark(ws->protocol);
        mprMark(ws->uri);
        ejsManagePot((EjsPot*) ws, flags);

    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->conn && ws->ejs->service) {
            httpDestroyConn(ws->conn);
            ws->conn = 0;
        }
    }
}


PUBLIC void ejsConfigureWebSocketType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "WebSocket"), sizeof(EjsWebSocket), 
            manageWebSocket, EJS_TYPE_POT)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, wsConstructor);
    ejsBindMethod(ejs, prototype, ES_WebSocket_bufferedAmount, ws_bufferedAmount);
    ejsBindMethod(ejs, prototype, ES_WebSocket_binaryType, ws_binaryType);
    ejsBindMethod(ejs, prototype, ES_WebSocket_close, ws_close);
    ejsBindMethod(ejs, prototype, ES_WebSocket_extensions, ws_extensions);
    ejsBindMethod(ejs, prototype, ES_WebSocket_off, ws_off);
    ejsBindMethod(ejs, prototype, ES_WebSocket_on, ws_on);
    ejsBindMethod(ejs, prototype, ES_WebSocket_protocol, ws_protocol);
    ejsBindMethod(ejs, prototype, ES_WebSocket_readyState, ws_readyState);
    ejsBindMethod(ejs, prototype, ES_WebSocket_send, ws_send);
    ejsBindMethod(ejs, prototype, ES_WebSocket_sendBlock, ws_sendBlock);
    ejsBindMethod(ejs, prototype, ES_WebSocket_url, ws_url);
    ejsBindMethod(ejs, prototype, ES_WebSocket_wait, ws_wait);
}
#endif /* BIT_HTTP_WEB_SOCKETS */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
