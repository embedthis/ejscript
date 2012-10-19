/**
    WebSocket.es -- WebSockets class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        WebSocket class to implement the WebSockets RFC 6455 specification for client-side communications. 

        WebSockets is a technology providing interactive communication between a server and client. Normal HTML connections
        follow a request / response paradigm and do not easily support asynchronous communications or unsolicited data
        pushed from the server to the client. WebSockets solves this by supporting bi-directional, full-duplex
        communications over persistent connections. A WebSocket connection is established over a standard HTTP connection
        and is then upgraded without impacting the original connection. This means it will work with existing networking
        infrastructure including firewalls and proxies.
        @spec ejs
        @stability prototype
        @see Http ByteArray
     */
    class WebSocket implements Stream {

        use default namespace public

        /** The $readyState value while waiting for the WebSockets open handshake to complete with the peer */
        static const CONNECTING = 0              

        /** The $readyState value once the WebSockets open handshake is complete and ready for communications */
        static const OPEN = 1

        /** The $readyState value when a closing handshake has commenced */
        static const CLOSING = 2

        /** The $readyState value when the WebSockets connection is closed */
        static const CLOSED = 3

        /**
            Create a new WebSocket and connect to the server using the supplied URI.
            @param uri URL to connect to
            @param protocols Optional set of protocols supported by the application. These are application-level
                protocols. This argument may be an array or a string of comma separated values.
         */
        native function WebSocket(uri: Uri, protocols = null)

        /**
            Event callback invoked when the WebSocket is ready to send and receive data. The $readyState will be set to OPEN.
          */
        var onopen: Function

        /**
            Event callback invoked when a message has been received from the server. The $readyState will be OPEN.
          */
        var onmessage: Function

        /**
            Event callback invoked when an error occurs.
          */
        var onerror: Function

        /**
            Event callback invoked when the connection is closed. The $readyState will be CLOSED.
          */
        var onclose: Function

        /**
            Type of binary data. Set to "ByteArray".
          */
        native function get binaryType(): String

        /**
            The number of bytes of buffered data to send
          */
        native function get bufferedAmount(): Number

        /**
            Send a close message and close the web socket connection
            @param code WebSocket status code to send to the peer explaining why the connection is being closed.
                Defaults to 1000 which means a successful, graceful closure.
            @param reason Optional reason string to indicate why the connection was closed. Must be less than 
                124 bytes of UTF-8 text in length.
         */
        native function close(code: Number = 1000, reason: String? = ""): Void

        /**
            Return the list of supported extensions
          */
        native function get extensions() : String

        /** 
            @duplicate Stream.off
            @hide
         */
        native function off(name, observer: Function): Void

        /** 
            @duplicate Stream.on
            All events are called with the following signature.  The "this" object will be set to the instance object
            if the callback is a method. Otherwise, "this" will be set to the Http instance. If Function.bind may also
            be used to define the "this" object and to inject additional callback arguments. 
                function (event: String, http: Http): Void
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes. Complete is always issued whether the request errors or not.
            @event error Issued if the request does not complete successfully. This is not issued if the request 
                ompletes successfully but with a non 200 Http status code.
            @hide
         */
        native function on(name, observer: Function): Http

        /**
            Return the application-level protocol selected by the server. Protocols are defined when 
            creating the $WebSocket.
          */
        native function get protocol() : String

        /**
            The readystate value. This value can be compared with the WebSocket constants: 
            $CONNECTING = 0, $OPEN = 1, $CLOSING = 2, $CLOSED = 3
         */
        native function get readyState() : Number

        /**
            Send data with the request.
            @param content Data to send with the request.
                If the content is a ByteArray, the message will be sent as a WebSocket Binary message.
                Otherwise, the message is converted to a String and sent as a WebSockets UTF-8 Text message.
               If multiple arguments are provided, each is sent as a separate message. 
         */
        native function send(...content): Void

        /**
            The URI provided to the constructor
         */
        native function get url(): Uri
    }
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
