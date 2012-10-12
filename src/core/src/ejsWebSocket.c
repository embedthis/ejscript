/**
    ejsWebSocket.c - WebSocket class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

#if BIT_WEB_SOCKETS
/*********************************** Forwards *********************************/

static void sendCloseEvent(Ejs *ejs, EjsWebSocket *ws);
static void sendErrorEvent(Ejs *ejs, EjsWebSocket *ws);
static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws);
static bool waitTillState(EjsWebSocket *ws, int state, MprTime timeout, int throw);
static void webSocketEvent(HttpConn *conn, int event, int arg);
static void webSocketNotify(HttpConn *conn, int state, int notifyFlags);

/************************************ Methods *********************************/
/*  
    function WebSocket(uri: Uri, protocols: String?)
 */
static EjsWebSocket *wsConstructor(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    ejsLoadHttpService(ejs);
    ws->ejs = ejs;

    //  MOB - should not require http service object. Or need httpGetHttp()

    if ((ws->conn = httpCreateConn(MPR->httpService, NULL, ejs->dispatcher)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    httpSetAsync(ws->conn, 1);
    if (argc >= 1) {
        ws->protocols = sclone(((EjsString*) argv[1])->value);
    }
    ws->uri = httpUriToString(((EjsUri*) argv[0])->uri, 0);
    httpPrepClientConn(ws->conn, 0);
    //  MOB - should get rid of
    httpSetConnNotifier(ws->conn, webSocketNotify);
    httpSetWebSocketProtocols(ws->conn, ws->protocols);
    httpSetWebSocketNotifier(ws->conn, webSocketEvent);
    httpSetConnContext(ws->conn, ws);
    startWebSocketRequest(ejs, ws);
    return ws;
}


/*
    function get binaryType(): String
 */
static EjsString *ws_binaryType(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, ws->dataType == WS_MSG_TEXT ? "text" : "binary");
}


/*  
    function get bufferedAmount(): Number
 */
static EjsNumber *ws_bufferedAmount(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    //  MOB - TODO
    return ejsCreateNumber(ejs, (MprNumber) 0);
}


/*  
    function close(code: Number, reason: String? = ""): Void
 */
static EjsObj *ws_close(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    if (ws->conn) {
        httpFinalize(ws->conn);
        sendCloseEvent(ejs, ws);
        httpDestroyConn(ws->conn);
        ws->conn = 0;
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
    //  MOB - must do onopen first
    if (conn->readq && conn->readq->count > 0) {
        ejsSendEvent(ejs, ws->emitter, "readable", NULL, ws);
    }
    if (!conn->connectorComplete && 
            !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        ejsSendEvent(ejs, ws->emitter, "writable", NULL, ws);
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
    return ejsCreateNumber(ejs, (MprNumber) 0);
}


/*  
    function send(content): Void
 */
static EjsString *ws_send(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    ssize   nbytes;

    if (ws->conn->state < HTTP_STATE_PARSED && !waitTillState(ws, HTTP_STATE_PARSED, -1, 1)) {
        return 0;
    }
    //  MOB - is this the best thing to do?
    ws->data = ejsCreateByteArray(ejs, -1);
    if (ejsWriteToByteArray(ejs, ws->data, 1, &argv[0]) < 0) {
        return 0;
    }
    nbytes = ejsGetByteArrayAvailableData(ws->data);
    if ((nbytes = httpSendBlock(ws->conn, WS_MSG_BINARY, (cchar*) &ws->data->value[ws->data->readPosition], nbytes)) < 0) {
        return 0;
    }
    return 0;
}


/*********************************** Support **********************************/

static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws)
{
    HttpConn        *conn;

    conn = ws->conn;
#if FUTURE
    if (ws->verifyPeer || ws->certFile) {
        if (!ws->ssl) {
            ws->ssl = mprCreateSsl();
        }
    }
    if (ws->ssl) {
        mprVerifySslPeer(ws->ssl, ws->verifyPeer);
        mprVerifySslIssuer(ws->ssl, ws->verifyPeer);
        if (ws->certFile) {
            mprSetSslCertFile(ws->ssl, ws->certFile);
        }
    }
#endif
    if (httpConnect(conn, "GET", ws->uri, ws->ssl) < 0) {
        ejsThrowIOError(ejs, "Can't issue request for \"%s\"", ws->uri);
        return 0;
    }
#if FUTURE
    ejsSendEvent(ejs, ws->emitter, "writable", NULL, ws);
#endif
    httpEnableConnEvents(ws->conn);
    return 0;
}


/*
    WebSocket callback
 */
static void webSocketEvent(HttpConn *conn, int event, int arg)
{
    Ejs             *ejs;
    EjsWebSocket    *ws;
    EjsByteArray    *ba;
    EjsAny          *data;
    EjsObj          *obj;
    char            *buf;
    ssize           len;

    ws = httpGetConnContext(conn);
    ejs = ws->ejs;
    
    if (event == HTTP_NOTIFY_READABLE) {
#if UNUSED
        //  MOB - would be nice to have HTTP_NOTIFY_OPEN
        if (!ws->opened) {
            ejsSendEvent(ejs, ws->emitter, "headers", NULL, ws);
            ws->opened = 1;
        }
#endif
        /* Called once per packet unless packet is too large (LimitWebSocketPacket) */
        print("wscontroller: READABLE format %s\n", arg == WS_MSG_TEXT ? "text" : "binary");
        len = httpGetReadCount(conn);
            //  MOB OPT
        if (arg == WS_MSG_TEXT) {
            if ((buf = mprAlloc(len + 1)) == 0) {
                //  MOB DIAG
                return;
            }
            if ((len = httpRead(conn, buf, len)) < 0) {
                //  MOB DIAG
                return;
            }
            data = ejsCreateStringFromBytes(ejs, buf, len);
        } else {
            if ((ba = ejsCreateByteArray(ejs, len)) == 0) {
                //  MOB DIAG
                return;
            }
            if ((len = httpRead(conn, (char*) ba->value, len)) < 0) {
                //  MOB DIAG
                return;
            }
            ejsSetByteArrayPositions(ejs, ba, -1, len);
            data = ba;
        }
        obj = ejsCreateObj(ejs, ESV(Object), 1);
        ejsSetPropertyByName(ejs, obj, EN("data"), data);
        ejsSendEvent(ejs, ws->emitter, "readable", obj, ws);

    } else if (event == HTTP_NOTIFY_CLOSED) {
        //  MOB - inline
        sendCloseEvent(ejs, ws);
        //  MOB - is this required
        httpFinalize(conn);

    } else if (event == HTTP_NOTIFY_ERROR) {
        sendErrorEvent(ejs, ws);
        //  MOB - is this required
        httpFinalize(conn);
    }
}


static bool verifyHandshake(HttpConn *conn)
{
    HttpRx          *rx;
    HttpTx          *tx;
    EjsWebSocket    *ws;
    cchar           *key, *expected;

    rx = conn->rx;
    tx = conn->tx;
    ws = httpGetConnContext(conn);

    if (rx->status != HTTP_CODE_SWITCHING) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket handshake status %d", rx->status);
        return 0;
    }
    if (!smatch(httpGetHeader(conn, "Connection"), "Upgrade")) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket Connection header");
        return 0;
    }
    if (!smatch(httpGetHeader(conn, "Upgrade"), "WebSocket")) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket Upgrade header");
        return 0;
    }
    expected = mprGetSHABase64(sjoin(tx->webSockKey, WEB_SOCKETS_MAGIC, NULL));
    key = httpGetHeader(conn, "Sec-WebSocket-Accept");
    if (!smatch(key, expected)) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket handshake key\n%s\n%s", key, expected);
        return 0;
    }
    ws->protocol = (char*) httpGetHeader(conn, "Sec-WebSocket-Protocol");
    return 1;
}



/*
    Connection callback
 */
static void webSocketNotify(HttpConn *conn, int state, int notifyFlags)
{
    Ejs             *ejs;
    EjsWebSocket    *ws;

    ws = httpGetConnContext(conn);
    ejs = ws->ejs;

    switch (state) {
    case HTTP_STATE_BEGIN:
        break;

    case HTTP_STATE_PARSED:
        if (verifyHandshake(conn)) {
            if (ws->emitter) {
                //  MOB - should be using web sockets callback
                //  MOB - SendEvent should allow null emitter
                ejsSendEvent(ejs, ws->emitter, "headers", NULL, ws);
            }
        }
        break;

    case HTTP_STATE_READY:
        if (ws && ws->emitter) {
            ejsSendEvent(ejs, ws->emitter, "open", NULL, ws);
        }
        break;

#if UNUSED
    case HTTP_STATE_CONTENT:
    case HTTP_STATE_RUNNING:
        break;

    case HTTP_STATE_COMPLETE:
        if (ws->emitter) {
            if (conn->error) {
                sendErrorEvent(ejs, ws);
            }
            sendCloseEvent(ejs, ws);
        }
        break;

    case 0:
        if (ws && ws->emitter) {
            if (notifyFlags & HTTP_NOTIFY_READABLE) {
                ejsSendEvent(ejs, ws->emitter, "readable", NULL, ws);
            } 
            if (notifyFlags & HTTP_NOTIFY_WRITABLE) {
                ejsSendEvent(ejs, ws->emitter, "writable", NULL, ws);
            }
        }
        break;
#endif
    }
}


#if UNUSED
/*  
    Read the required number of bytes into the response content buffer. Count < 0 means transfer the entire content.
    Returns the number of bytes read.
 */ 
static ssize readWebSocketData(Ejs *ejs, EjsWebSocket *ws, ssize count)
{
    MprBuf          *buf;
    WebSocketConn   *conn;
    ssize           len, space, nbytes;

    conn = ws->conn;
    buf = ws->responseContent;
    mprResetBufIfEmpty(buf);
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
        if (ws->conn->async || (nbytes == 0 && conn->state >= HTTP_STATE_COMPLETE)) {
            break;
        }
    }
    if (count < 0) {
        return mprGetBufLength(buf);
    }
    return min(count, mprGetBufLength(buf));
}
#endif


static void sendCloseEvent(Ejs *ejs, EjsWebSocket *ws)
{
    if (!ws->closed && ejs->service) {
        ws->closed = 1;
        if (ws->emitter) {
            ejsSendEvent(ejs, ws->emitter, "close", NULL, ws);
        }
    }
}


static void sendErrorEvent(Ejs *ejs, EjsWebSocket *ws)
{
    if (!ws->error) {
        ws->error = 1;
        if (ws->emitter) {
            ejsSendEvent(ejs, ws->emitter, "error", NULL, ws);
        }
    }
}


static bool waitTillState(EjsWebSocket *ws, int state, MprTime timeout, int throw)
{
    Ejs             *ejs;
    MprTime         mark, remaining;
    HttpConn        *conn;
    HttpUri         *uri;
    char            *url;
    int             count, redirectCount, success, rc;

    mprAssert(state >= HTTP_STATE_PARSED);

    ejs = ws->ejs;
    conn = ws->conn;
    mprAssert(conn->state >= HTTP_STATE_CONNECTED);

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
    mark = mprGetTime();
    remaining = timeout;
    while (conn->state < state && count <= conn->retries && redirectCount < 16 && 
           !conn->error && !ejs->exiting && !mprIsStopping(conn)) {
        count++;
        if ((rc = httpWait(conn, HTTP_STATE_PARSED, remaining)) == 0) {
            if (httpNeedRetry(conn, &url)) {
                if (url) {
                    uri = httpCreateUri(url, 0);
                    httpCompleteUri(uri, httpCreateUri(ws->uri, 0), 0);
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
                httpFormatError(conn, HTTP_CODE_COMMS_ERROR, "Connection error");
            } else if (rc == MPR_ERR_TIMEOUT) {
                if (timeout > 0) {
                    httpFormatError(conn, HTTP_CODE_REQUEST_TIMEOUT, "Request timed out");
                }
            } else {
                httpFormatError(conn, HTTP_CODE_NO_RESPONSE, "Client request error");
            }
            break;
        }
        if (conn->rx) {
            if (conn->rx->status == HTTP_CODE_REQUEST_TOO_LARGE || 
                    conn->rx->status == HTTP_CODE_REQUEST_URL_TOO_LARGE) {
                /* No point retrying */
                break;
            }
        }
        if (timeout > 0) {
            remaining = mprGetRemainingTime(mark, timeout);
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


/*********************************** Factory **********************************/

/*  
    Manage the object properties for the garbage collector
 */
static void manageWebSocket(EjsWebSocket *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->conn);
        mprMark(ws->emitter);
        mprMark(ws->data);
        mprMark(ws->ssl);
        mprMark(ws->protocols);
        mprMark(ws->protocol);
        mprMark(ws->uri);
        mprMark(TYPE(ws));

    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->conn) {
            sendCloseEvent(ws->ejs, ws);
            httpDestroyConn(ws->conn);
            ws->conn = 0;
        }
    }
}


void ejsConfigureWebSocketType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "WebSocket"), sizeof(EjsWebSocket), 
            manageWebSocket, EJS_TYPE_OBJ)) == 0) {
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
}

#endif
/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
