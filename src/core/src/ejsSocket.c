/**
    ejsSocket.c - Socket class. This implements TCP/IP v4 and v6 connectivity.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Forwards *********************************/

static void enableSocketEvents(EjsSocket *sp, int (*proc)(EjsSocket *sp, MprEvent *event));
#if UNUSED
static int socketConnectEvent(EjsSocket *sp, MprEvent *event);
#endif
static int socketIOEvent(EjsSocket *sp, MprEvent *event);
static int socketListenEvent(EjsSocket *listen, MprEvent *event);

/************************************ Methods *********************************/
/*
    function Socket()
 */
static EjsSocket *sock_Socket(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    //  TODO -- ssl?
    sp->ejs = ejs;
    sp->sock = mprCreateSocket(NULL);
    if (sp->sock == 0) {
        ejsThrowMemoryError(ejs);
        return 0;
    }
#if UNUSED
    sp->waitHandler.fd = -1;
#endif
    return sp;
}


/*
    function accept(): Socket
 */
EjsSocket *sock_accept(Ejs *ejs, EjsSocket *listen, int argc, EjsObj **argv)
{
    MprSocket   *sock;
    EjsSocket   *sp;

    if ((sock = mprAcceptSocket(listen->sock)) == 0) {
        ejsThrowIOError(ejs, "Can't accept new socket");
        return 0;
    }
    sp = ejsCreateSocket(ejs);
    sp->sock = sock;
    sp->async = listen->async;
    if (sp->async) {
        sp->mask |= MPR_READABLE;
        enableSocketEvents(sp, socketIOEvent);
    } else {
        mprSetSocketBlockingMode(sp->sock, 1);
    }
    return sp;
}


/*
    function get address(): String
 */
EjsString *sock_address(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return ejsCreateStringFromAsc(ejs, sp->address);
}


/*
    function get async(): Boolean
 */
EjsBoolean *sock_async(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    return (sp->async) ? S(true) : S(false);
}


/*
    function set async(enable: Boolean): Void
 */
EjsObj *sock_set_async(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    sp->async = (argv[0] == S(true));
    return 0;
}


/*
    function close(): Void
    TODO - should support graceful option
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
    char            *cp;

    address = (EjsString*) argv[0];
    if (ejsIs(ejs, address, Number)) {
        sp->address = sclone("127.0.0.1");
        sp->port = (int) ((EjsNumber*) address)->value;
    } else {
        if (!ejsIs(ejs, address, String)) {
            address = ejsToString(ejs, address);
        }
        sp->address = ejsToMulti(ejs, address);
        if ((cp = strchr(sp->address, ':')) != 0) {
            *cp++ = '\0';
            sp->port = atoi(cp);
        } else {
            ejsThrowArgError(ejs, "Address must have a port");
            return 0;
        }
    }
    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    if (mprConnectSocket(sp->sock, sp->address, sp->port, 0) < 0) {
        ejsThrowArgError(ejs, "Can't open client socket");
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
    function listen(address): Void
    @param address Can be either a "ip", "ip:port" or port
 */
static EjsObj *sock_listen(Ejs *ejs, EjsSocket *sp, int argc, EjsObj **argv)
{
    EjsString   *address;
    char        *cp;

    address = (EjsString*) argv[0];
    if (ejsIs(ejs, address, Number)) {
        sp->address = sclone("");
        sp->port = (int) ((EjsNumber*) address)->value;
    } else {
        if (!ejsIs(ejs, address, String)) {
            address = ejsToString(ejs, address);
        }
        sp->address = ejsToMulti(ejs, address);
        if ((cp = strchr(sp->address, ':')) != 0) {
            *cp++ = '\0';
            sp->port = atoi(cp);
        } else {
            ejsThrowArgError(ejs, "Address must have a port");
            return 0;
        }
    }
    if (!sp->sock) {
        ejsThrowStateError(ejs, "Socket is closed");
        return 0;
    }
    if (mprListenOnSocket(sp->sock, sp->address, sp->port, 0) < 0) {
        ejsThrowArgError(ejs, "Can't open listening socket");
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
    function on(name: [String|Array], observer: Function): Void
 */
EjsObj *sock_on(Ejs *ejs, EjsSocket *sp, int argc, EjsAny **argv)
{
    ejsAddObserver(ejs, &sp->emitter, argv[0], argv[1]);
    return 0;
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
        //  Do something
    }
    nbytes = mprReadSocket(sp->sock, &ba->value[offset], count);
    if (nbytes < 0) {
        //  TODO -- should not be using EOF event
        ejsSendEvent(ejs, sp->emitter, "eof", NULL, sp);
        //  TODO - do we need to set the mask here?
        return S(null);
    }
    if (nbytes == 0) {
        //  TODO - but in async, this does not mean eof. See mpr for how to tell eof
        //  TODO -- should not be using EOF event
        ejsSendEvent(ejs, sp->emitter, "eof", NULL, sp);
        //  TODO - do we need to set the mask here?
        return S(null);
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

    ba = sp->data;
    nbytes = 0;
    count = 0;
    if (ba && (count = ejsGetByteArrayAvailable(ba)) > 0) {
        nbytes = mprWriteSocket(sp->sock, &ba->value[ba->readPosition], count);
        if (nbytes < 0) {
            ejsThrowIOError(ejs, "Can't write to socket");
            return 0;
        }
        ba->readPosition += nbytes;
    }
    if (ejsGetByteArrayAvailable(ba) == 0) {
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
        ejsResetByteArray(sp->data);
    } else {
        sp->data = ejsCreateByteArray(ejs, -1);
    } 
    //  TODO - OPT. Could not copy and write directly from original source if source is a byte array
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
    if (sp->sock->handler == 0) {
        mprAddSocketHandler(sp->sock, sp->mask, ejs->dispatcher, (MprEventProc) proc, sp, 0);
    } else {
        mprEnableSocketEvents(sp->sock, sp->mask);
    }
}


#if UNUSED
static int socketConnectEvent(EjsSocket *sp, MprEvent *event)
{
    Ejs     *ejs;

    ejs = sp->ejs;
    if (sp->emitter) {
        ejsSendEvent(ejs, sp->emitter, "connect", NULL, sp);
        ejsSendEvent(ejs, sp->emitter, "writable", NULL, sp);
    }
    enableSocketEvents(sp, socketIOEvent);
    return 0;
}
#endif


static int socketListenEvent(EjsSocket *listen, MprEvent *event)
{
    Ejs     *ejs;

    ejs = listen->ejs;
    if (listen->emitter) {
        ejsSendEvent(ejs, listen->emitter, "accept", NULL, listen);
    }
    enableSocketEvents(listen, socketListenEvent);
    return 0;
}


static int socketIOEvent(EjsSocket *sp, MprEvent *event)
{
    Ejs     *ejs;

    ejs = sp->ejs;
    if (event->mask & MPR_READABLE) {
        if (sp->emitter) {
            ejsSendEvent(ejs, sp->emitter, "readable", NULL, sp);
        }
        sp->mask |= MPR_READABLE;
    } 
    if (event->mask & MPR_WRITABLE) {
        writeSocketData(ejs, sp);
    }
    enableSocketEvents(sp, socketIOEvent);
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
    }
}


EjsSocket *ejsCreateSocket(Ejs *ejs)
{
    EjsSocket   *sp;

    sp = ejsCreateObj(ejs, ejsGetTypeByName(ejs, N(EJS_EJS_NAMESPACE, "Socket")), 0);
    sp->ejs = ejs;
#if UNUSED
    sp->waitHandler.fd = -1;
#endif
    return sp;
}


void ejsConfigureSocketType(Ejs *ejs)
{
    EjsType     *type;
    EjsPot      *prototype;

    type = ejsConfigureNativeType(ejs, N("ejs", "Socket"), sizeof(EjsSocket), (MprManager) manageSocket, EJS_OBJ_HELPERS);
    type->mutableInstances = 1;
    prototype = type->prototype;

    ejsBindConstructor(ejs, type, sock_Socket);
    ejsBindMethod(ejs, prototype, ES_Socket_accept, sock_accept);
    ejsBindMethod(ejs, prototype, ES_Socket_address, sock_address);
    ejsBindAccess(ejs, prototype, ES_Socket_async, sock_async, sock_set_async);
    ejsBindMethod(ejs, prototype, ES_Socket_close, sock_close);
    ejsBindMethod(ejs, prototype, ES_Socket_connect, sock_connect);
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
