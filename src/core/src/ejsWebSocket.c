/**
    ejsWebSocket.c - WebSocket class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static void relayEvent(EjsWebSocket *ws, int event, EjsAny *data);
static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws);
static bool waitTillState(EjsWebSocket *ws, int state, MprTime timeout, int throw);
static void webSocketNotify(HttpConn *conn, int state, int notifyFlags);

/************************************ Methods *********************************/
/*  
    function WebSocket(uri: Uri, protocols = null)
 */
static EjsWebSocket *wsConstructor(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    assure(ejsIsPot(ejs, ws));

    ejsLoadHttpService(ejs);
    ws->ejs = ejs;

    if ((ws->conn = httpCreateConn(MPR->httpService, NULL, ejs->dispatcher)) == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    httpSetAsync(ws->conn, 1);
    if (argc >= 1) {
        if (ejsIs(ejs, argv[1], Array)) {
            ws->protocols = sclone((ejsToString(ejs, argv[1]))->value);
        } else if (ejsIs(ejs, argv[1], String)) {
            ws->protocols = sclone(((EjsString*) argv[1])->value);
        }
    }
    ws->uri = httpUriToString(((EjsUri*) argv[0])->uri, 0);
    httpPrepClientConn(ws->conn, 0);
    httpSetConnNotifier(ws->conn, webSocketNotify);
    httpSetWebSocketProtocols(ws->conn, ws->protocols);
    httpSetConnContext(ws->conn, ws);
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
    return ejsCreateNumber(ejs, ws->conn->writeq->count);
}


/*  
    function close(code: Number = 1000, reason: String? = ""): Void
 */
static EjsObj *ws_close(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    HttpConn    *conn;
    char        *reason;
    int         status;

    conn = ws->conn;
    if (conn) {
        httpFinalize(conn);
        status = ejsGetInt(ejs, argv[0]);
        reason = (argv[1] != ESV(null) && argv[1] != ESV(undefined)) ? ejsToMulti(ejs, argv[1]): 0; 
        httpSendClose(conn, status, reason);
#if UNUSED
        relayEvent(ws, HTTP_EVENT_APP_CLOSE, 0);
        httpDestroyConn(conn);
        ws->conn = 0;
#endif
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
        relayEvent(ws, HTTP_EVENT_READABLE, 0);
    }
    if (!conn->tx->connectorComplete && 
            !conn->error && HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE &&
            conn->writeq->ioCount == 0) {
        relayEvent(ws, HTTP_EVENT_WRITABLE, 0);
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
    return ejsCreateNumber(ejs, (MprNumber) ws->conn->rx->webSockState);
}


/*  
    function send(...content): Void
 */
static EjsString *ws_send(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    EjsArray        *args;
    EjsByteArray    *ba;
    EjsAny          *arg;
    ssize           nbytes;
    int             i;

    args = (EjsArray*) argv[0];
    if (ws->conn->state < HTTP_STATE_PARSED && !waitTillState(ws, HTTP_STATE_PARSED, -1, 1)) {
        return 0;
    }
    for (i = 0; i < args->length; i++) {
        if ((arg = ejsGetProperty(ejs, args, i)) == 0) {
            if (ejsIs(ejs, arg, ByteArray)) {
                ba = (EjsByteArray*) arg;
                nbytes = ejsGetByteArrayAvailableData(ba);
                nbytes = httpSendBlock(ws->conn, WS_MSG_BINARY, (cchar*) &ba->value[ba->readPosition], nbytes);
            } else {
                nbytes = httpSend(ws->conn, ejsToMulti(ejs, arg));
            }
            if (nbytes < 0) {
                return 0;
            }
        }
    }
    return 0;
}


/*  
    function get url(): Uri
 */
static EjsUri *ws_url(Ejs *ejs, EjsWebSocket *ws, int argc, EjsObj **argv)
{
    return ejsCreateUriFromAsc(ejs, ws->uri);
}

/*********************************** Support **********************************/

static EjsObj *startWebSocketRequest(Ejs *ejs, EjsWebSocket *ws)
{
    HttpConn        *conn;

    conn = ws->conn;
#if MOB && FUTURE
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
    httpEnableConnEvents(ws->conn);
    return 0;
}


static void relayEvent(EjsWebSocket *ws, int event, EjsAny *data)
{
    Ejs             *ejs;
    EjsAny          *eobj;
    EjsFunction     *fn;
    HttpRx          *rx;
    cchar           *eventName;
    int             slot;

    ejs = ws->ejs;
    rx = ws->conn->rx;
    eobj = ejsCreateObj(ejs, ESV(Object), 0);
    slot = -1;
    eventName = 0;

    switch(event) {
    case HTTP_EVENT_READABLE:
        slot = ES_WebSocket_onmessage;
        eventName = "readable";
        assure(data);
        ejsSetPropertyByName(ejs, eobj, EN("data"), data);
        break;

    case HTTP_EVENT_ERROR:
        eventName = "error";
        slot = ES_WebSocket_onerror;
        break;

    case HTTP_EVENT_APP_OPEN:
        slot = ES_WebSocket_onopen;
        eventName = "headers";
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
        ejsSetPropertyByName(ejs, eobj, EN("code"), ejsCreateNumber(ejs, rx->closeStatus));
        ejsSetPropertyByName(ejs, eobj, EN("reason"), ejsCreateStringFromAsc(ejs, rx->closeReason));
        ejsSetPropertyByName(ejs, eobj, EN("wasClean"), ejsCreateBoolean(ejs, rx->closeStatus != WS_STATUS_COMMS_ERROR));
        break;
    }
    if (slot >= 0) {
        if (ws->emitter) {
            ejsSendEvent(ejs, ws->emitter, eventName, ws, data);
        }
        fn = ejsGetProperty(ejs, ws, slot);
        if (ejsIsFunction(ejs, fn)) {
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
    char            *buf;
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
            mprLog(4, "Web socket protocol %s", ws->protocol);
            relayEvent(ws, HTTP_EVENT_APP_OPEN, 0);
        }
        break;
    
    case HTTP_EVENT_READABLE:
        /* Called once per packet unless packet is too large (LimitWebSocketPacket) */
        print("wscontroller: READABLE format %s\n", arg == WS_MSG_TEXT ? "text" : "binary");
        len = httpGetReadCount(conn);
        if (arg == WS_MSG_TEXT) {
            if ((buf = mprAlloc(len + 1)) == 0) {
                return;
            }
            if ((len = httpRead(conn, buf, len)) < 0) {
                ejsThrowIOError(ejs, "Can't read packet");
                return;
            }
            data = ejsCreateStringFromBytes(ejs, buf, len);
        } else {
            if ((ba = ejsCreateByteArray(ejs, len)) == 0) {
                return;
            }
            if ((len = httpRead(conn, (char*) ba->value, len)) < 0) {
                ejsThrowIOError(ejs, "Can't read packet");
                return;
            }
            ejsSetByteArrayPositions(ejs, ba, -1, len);
            data = ba;
        }
        relayEvent(ws, event, data);
        break;

    case HTTP_EVENT_ERROR:
        if (!ws->error && !ws->closed) {
            ws->error = 1;
            relayEvent(ws, event, 0);
        }
        break;

    case HTTP_EVENT_APP_CLOSE:
        if (!ws->closed) {
            ws->closed = 1;
            relayEvent(ws, event, 0);
        }
        break;
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
#if UNUSED
        mprMark(ws->data);
#endif
        mprMark(ws->ssl);
        mprMark(ws->protocols);
        mprMark(ws->protocol);
        mprMark(ws->uri);
        ejsManagePot((EjsPot*) ws, flags);

    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->conn && ws->ejs->service) {
            if (!ws->closed) {
                relayEvent(ws, HTTP_EVENT_APP_CLOSE, 0);
            }
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
#if ES_WebSocket_url
    ejsBindMethod(ejs, prototype, ES_WebSocket_url, ws_url);
#endif
}

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
