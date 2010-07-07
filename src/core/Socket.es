/*
    Socket.es -- Socket I/O class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Client and server side TCP/IP support for IPv4 and IPv6 communications. This class supports broadcast, datagram and
        byte-stream socket sevices for both client and server endpoints. Aynchronous and asynchronous operation is supported.
        @spec ejs
        @stability prototype
     */
    class Socket implements Stream {

        use default namespace public

        //  MOB -- SSL

        /** 
            Create a socket object
         */
        native function Socket()

        /** 
            @duplicate Stream.observe 
            @event readable Issued when the response headers have been fully received and some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
         */
        native function observe(name, observer: Function): Void

//  MOB - or would it be better to have the accepted socket passed in as a callback parameter?
        /** 
            Receive a client socket in response to a "connect" event. The accept call must be called after invoking
            $listen and receiving a client connection.
            @returns A socket connected to the client endpoint.
         */
        native function accept(): Socket

        /** 
            Local IP address bound to this socket. Set to the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

        /** @duplicate Stream.async */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        /** 
            Establish a connection to a client from this socket to the supplied address. After a successful call to 
            connect() the socket may be used for sending and receiving.
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @throws IOError if the connection fails. Reasons may include the socket is already bound or the host is unknown.
            @events Issues a "connect" event when the connection is complete.
         */
        native function connect(address: Object): Void

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8"
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** @duplicate Stream.flush */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Listen on a socket for client connections. This will put the socket into a server role for communcations.
            If the socket is in sync mode, the listen call will block until a client connection is received and
            the call will return the client socket. 
            If a the listening socket is in async mode, the listen call will return immediately and 
            client connections will be notified via "accept" events. 
            In this case, when a client connection is received, the $accept function must be called to 
            receive the client socket object for the connection. 
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @return A client socket if in sync mode. No return value if in async mode.
            @throws ArgError if the specified listen address is not valid, and IOError for network errors.
            @event Issues a "accept" event when there is a new connection available. In response, the $accept method
                should be called.
         */
        native function listen(address): Socket

        /** 
            The port bound to this socket. Set to the integer port number or zero if not bound.
         */
        native function get port(): Number 

        /** @duplicate Stream.read */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        //  MOB - readString, readBytes, readXML

        /** 
            The remote address bound to this socket. Set to the remote address in dot notation or empty string if it 
            is not bound.
         */
        native function get remoteAddress(): String 

        /** @duplicate Stream.removeObserver */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            @duplicate Stream.write 
         */
        //  MOB -- or ...data
        native function write(...items): Number
    }
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
