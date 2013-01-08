/**
    ejsSocket.c - Socket class. This implements TCP/IP v4 and v6 connectivity.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static void enableSocketEvents(EjsSocket *sp, int (*proc)(EjsSocket *sp, MprEvent *event));
static int socketIOEvent(EjsSocket *sp, MprEvent *event);
static int socketListenEvent(EjsSocket *listen, MprEvent *event);

/************************************ Methods *********************************/
/*
    function Socket()
 */
static EjsSocket *sock_Socket(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    sp->ejs = ejs;
    sp->sock = mprCreateSocket(NULL);
    if (sp->sock == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
    return sp;
}


/*
    function accept(): Socket
 */
PUBLIC EjsSocket *sock_accept(Ejs *ejs, EjsSocket *listen, int argc, EjsObj **argv)
{
    MprSocket   *sock;
    EjsSocket   *sp;

    if ((sock = mprAcceptSocket(listen->sock)) == 0) {
        ejsThrowIOError(ejs, "Cannot accept new socket");
        return 0;
    }
    if ((sp = ejsCreateSocket(ejs, sock, listen->async)) != 0) {
        if (sp->async) {
            sp->mask |= MPR_READABLE;
            enableSocketEvents(sp, socketIOEvent);
        } else {
            mprSetSocketBlockingMode(sp->sock, 1);
        }
    }
    return sp;
}


/*
    function get address(): String
 */
PUBLIC EjsString *sock_address(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, sp->address);
}


/*
    function get async(): Boolean
 */
PUBLIC EjsBoolean *sock_async(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return (sp->async) ? ESV(true) : ESV(false);
}


/*
    function set async(enable: Boolean): Void
 */
PUBLIC EjsObj *sock_set_async(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    sp->async = (argv[0] == ESV(true));
    return 0;
}


/*
    function close(): Void
 */
static EjsObj *sock_close(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    ejsSendEvent(ejs, sp->emitter, "close", NULL, sp);
    if (sp->sock) {
        mprCloseSocket(sp->sock, 0);
        sp->sock = 0;
    }
    return 0;
}


/*
    function connect(address): Void
 */
static EjsObj *sock_connect(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    EjsString       *address;

    address = (EjsString*) argv[0];
    if (ejsIs(ejs, address, Number)) {
        sp->address = sclone("127.0.0.1");
        sp->port = (int) ((EjsNumber*) address)->value;
    } else {
        if (!ejsIs(ejs, address, String)) {
            address = ejsToString(ejs, address);
        }
        sp->address = ejsToMulti(ejs, address);
        mprParseSocketAddress(sp->address, &sp->address, &sp->port, NULL, 0);
        if (sp->address == 0) {
            sp->address = sclone("127.0.0.1");
        }
        if (sp->port == 0) {
            ejsThrowArgError(ejs, "Address must have a port");
            return 0;
        }
    }
    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    if (mprConnectSocket(sp->sock, sp->address, sp->port, 0) < 0) {
        ejsThrowArgError(ejs, "Cannot open client socket");
        return 0;
    }
    if (sp->async) {
        sp->mask |= MPR_READABLE;
        enableSocketEvents(sp, socketIOEvent);
    } else {
        mprSetSocketBlockingMode(sp->sock, 1);
    }
    ejsSendEvent(ejs, sp->emitter, "writable", NULL, sp);
    return 0;
}


/**
    function get isEof(): Boolean
 */
static EjsObj *sock_isEof(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return (sp->sock == 0 || mprIsSocketEof(sp->sock)) ? ESV(true) : ESV(false);
}


/**
    function listen(address): Void
    @param address Can be either a "ip", "ip:port" or port
 */
static EjsObj *sock_listen(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    EjsString   *address;

    address = (EjsString*) argv[0];
    if (ejsIs(ejs, address, Number)) {
        sp->address = sclone("");
        sp->port = (int) ((EjsNumber*) address)->value;
    } else {
        if (!ejsIs(ejs, address, String)) {
            address = ejsToString(ejs, address);
        }
        sp->address = ejsToMulti(ejs, address);
        //  MOB - should listen to secure and permit https://IP:PORT
        mprParseSocketAddress(sp->address, &sp->address, &sp->port, NULL, 80);
    }
    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    if (mprListenOnSocket(sp->sock, sp->address, sp->port, 0) < 0) {
        ejsThrowArgError(ejs, "Cannot open listening socket");
        return 0;
    }
    if (sp->async) {
        sp->mask |= MPR_READABLE;
        enableSocketEvents(sp, socketListenEvent);
    } else {
        mprSetSocketBlockingMode(sp->sock, 1);
    }
    return 0;
}


/*
    function off(name, observer: Function): Void
 */
static EjsObj *sock_off(Ejs *ejs, EjsSocket *sp, int argc, EjsAny **argv)
{
    ejsRemoveObserver(ejs, sp->emitter, argv[0], argv[1]);
    return 0;
}


/*
    function on(name: [String|Array], observer: Function): Socket
 */
PUBLIC EjsSocket *sock_on(Ejs *ejs, EjsSocket *sp, int argc, EjsAny **argv)
{
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
    return sp;
}


/*
    function get port(): Number
 */
static EjsNumber *sock_port(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, sp->port);
}


/*
    function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
 */
static EjsNumber *sock_read(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    EjsByteArray    *ba;
    ssize           nbytes, offset, count;

    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    ba = (EjsByteArray*) argv[0];
    offset = (argc >= 2) ? ejsGetInt(ejs, argv[1]) : 0;
    count = (argc >= 3) ? ejsGetInt(ejs, argv[2]) : -1;

    if (offset < 0) {
        offset = ba->writePosition;
    }
    if (count < 0) {
        count = ba->size - offset;
    }
    if (count <= 0) {
        return ESV(zero);
    }
    nbytes = mprReadSocket(sp->sock, &ba->value[offset], count);
    if (nbytes < 0) {
        /* If async, Caller must test "isEof" to determine if eof or error */
        return ESV(null);
    }
    ba->writePosition += nbytes;
    sp->mask |= MPR_READABLE;
    enableSocketEvents(sp, socketIOEvent);
    return ejsCreateNumber(ejs, (int) nbytes);
}


/*
    function get remoteAddress(): String
 */
static EjsString *sock_remoteAddress(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, sp->address);
}


static ssize writeSocketData(Ejs *ejs, EjsSocket *sp)
{
    EjsByteArray    *ba;
    ssize           nbytes, count;

    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    ba = sp->data;
    nbytes = 0;
    count = 0;
    if (ba && (count = ejsGetByteArrayAvailableData(ba)) > 0) {
        nbytes = mprWriteSocket(sp->sock, &ba->value[ba->readPosition], count);
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Cannot write to socket");
            return 0;
        }
        ba->readPosition += nbytes;
    }
    if (ejsGetByteArrayAvailableData(ba) == 0) {
        if (sp->emitter) {
            ejsSendEvent(ejs, sp->emitter, "writable", NULL, sp);
        }
        if (sp->async) {
            sp->mask &= ~MPR_WRITABLE;
        }
    } else if (sp->async) {
        if (nbytes < count) {
            sp->mask |= MPR_WRITABLE;
        } else {
            sp->mask &= ~MPR_WRITABLE;
        }
    }
    return nbytes;
}


/*
    function write(...data): Number
 */
static EjsNumber *sock_write(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    ssize     nbytes;

    if (sp->data) {
        /* Reset pointers if empty */
        ejsResetByteArray(ejs, sp->data);
    } else {
        sp->data = ejsCreateByteArray(ejs, -1);
    } 
    if (ejsWriteToByteArray(ejs, sp->data, 1, &argv[0]) < 0) {
        return 0;
    }
    if ((nbytes = writeSocketData(ejs, sp)) < 0) {
        return 0;
    }
    if (sp->async) {
        enableSocketEvents(sp, socketIOEvent);
    }
    return ejsCreateNumber(ejs, (MprNumber) nbytes);
}


/*********************************** Support **********************************/

static void enableSocketEvents(EjsSocket *sp, int (*proc)(EjsSocket *sp, MprEvent *event))
{
    Ejs     *ejs;

    ejs = sp->ejs;
    assert(sp->sock);
    
    if (sp->sock->handler == 0) {
        mprAddSocketHandler(sp->sock, sp->mask, ejs->dispatcher, (MprEventProc) proc, sp, 0);
    } else {
        mprEnableSocketEvents(sp->sock, sp->mask);
    }
}


static int socketListenEvent(EjsSocket *listen, MprEvent *event)
{
    Ejs     *ejs;

    ejs = listen->ejs;
    if (listen->emitter) {
        ejsSendEvent(ejs, listen->emitter, "accept", NULL, listen);
    }
    if (listen->sock) {
        enableSocketEvents(listen, socketListenEvent);
    }
    return 0;
}


static int socketIOEvent(EjsSocket *sp, MprEvent *event)
{
    Ejs     *ejs;

    ejs = sp->ejs;
    if (sp->sock) {
        if (event->mask & MPR_READABLE) {
            if (sp->emitter) {
                ejsSendEvent(ejs, sp->emitter, "readable", NULL, sp);
            }
            sp->mask |= MPR_READABLE;
        } 
        if (event->mask & MPR_WRITABLE) {
            writeSocketData(ejs, sp);
        }
        if (sp->sock) {
            enableSocketEvents(sp, socketIOEvent);
        }
    }
    return 0;
}


/*********************************** Factory **********************************/
/*  
   Manage the object properties for the garbage collector
 */
static void manageSocket(EjsSocket *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->emitter);
        mprMark(sp->data);
        mprMark(sp->sock);
        mprMark(sp->address);
        mprMark(sp->mutex);
    }
}


PUBLIC EjsSocket *ejsCreateSocket(Ejs *ejs, MprSocket *sock, bool async)
{
    EjsSocket   *sp;

    sp = ejsCreateObj(ejs, ejsGetTypeByName(ejs, N(EJS_EJS_NAMESPACE, "Socket")), 0);
    sp->ejs = ejs;
    sp->sock = sock;
    sp->async = async;
    return sp;
}


PUBLIC void ejsConfigureSocketType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    if ((type = ejsFinalizeScriptType(ejs, N("ejs", "Socket"), sizeof(EjsSocket), manageSocket,
            EJS_TYPE_OBJ | EJS_TYPE_MUTABLE_INSTANCES)) == 0) {
        return;
    }
    prototype = type->prototype;
    ejsBindConstructor(ejs, type, sock_Socket);
    ejsBindMethod(ejs, prototype, ES_Socket_accept, sock_accept);
    ejsBindMethod(ejs, prototype, ES_Socket_address, sock_address);
    ejsBindAccess(ejs, prototype, ES_Socket_async, sock_async, sock_set_async);
    ejsBindMethod(ejs, prototype, ES_Socket_close, sock_close);
    ejsBindMethod(ejs, prototype, ES_Socket_connect, sock_connect);
    ejsBindMethod(ejs, prototype, ES_Socket_isEof, sock_isEof);
    ejsBindMethod(ejs, prototype, ES_Socket_listen, sock_listen);
    ejsBindMethod(ejs, prototype, ES_Socket_off, sock_off);
    ejsBindMethod(ejs, prototype, ES_Socket_on, sock_on);
    ejsBindMethod(ejs, prototype, ES_Socket_port, sock_port);
    ejsBindMethod(ejs, prototype, ES_Socket_read, sock_read);
    ejsBindMethod(ejs, prototype, ES_Socket_remoteAddress, sock_remoteAddress);
    ejsBindMethod(ejs, prototype, ES_Socket_write, sock_write);
}

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
