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
        @stability evolving
        @see ByteArray Http
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

        /**< Continuation of WebSocket message */
        static const MSG_CONT = 0x0

        /**< WebSocket text message*/
        static const MSG_TEXT = 0x1

        /**< WebSocket binary message */
        static const MSG_BINARY = 0x2

        /**< WebSocket close message */
        static const MSG_CLOSE = 0x8

        /**< WebSocket ping message */
        static const MSG_PING = 0x9

        /**< WebSocket pong message */
        static const MSG_PONG = 0xA


        /**
            Create a new WebSocket and connect to the server using the supplied URI.
            @param uri URL to connect to
            @param protocols Optional set of protocols supported by the application. These are application-level
                protocols. This argument may be an array or a string of comma separated values.
            @param options
            @options certificate Path|String Optional path to file containing certificates to validate the 
                server certificate.
            @options verify Boolean Set to true to verify the server certificate. 
                The certificate is verified and the issuer chain is verified. Defaults to true.
                Set to false to accept a self-signed certificate.
            @options frames Preserve individual message frames to onmessage.
         */
        native function WebSocket(uri: Uri, protocols = null, options = null)

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
            The event has properties: "data" containing the message, "last" set to true if this is the last portion of data for
                a logical message, and "type" set to the WebSockets message type.
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
            creating the $WebSocket
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
               The message is buffered and so this call will never fail. Sending very large messages can consume
               a lot of memory. Use $sendBlock to efficiently send large messages.
                @return Count of bytes written. Returns null if the WebSocket is closed.
               @throws 
         */
        native function send(...content): Number

        /* 
            WARNING: these must match the equivalent http.h definitions 
         */
        /**< sendBlock mode to not block and absorb only what can fit into the queue */
        static const NON_BLOCK = 0x1

        /**< sendBlock mode to buffer the content */
        static const BUFFER = 0x2

        /**< sendBlock mode to block waiting for the content to be sent */
        static const BLOCK = 0x4

        /**
            Send a block of data with options to control message framing and buffering. 
            This API is useful for sending multipart or large messages, or wherever you need to control the framing of 
            WebSockets messages. 
            @param content The content may be a ByteArray or any other type which is then converted to a string.
            @param options
            @options last Set to false to indicate there is more data to complete this message. Set to true for the last portion
            of the message. Defaults to true if unspecified.
            @options type Message type. Masked to the valid WebSockets message types. Defaults to Text.
            @options mode API transmission mode. Set to BUFFER to buffer the entire contents before transmitting. Set to BLOCK
            to have the API wait until all the content is accepted. Set to NON_BLOCK for the API to accept whatever data can fit
            into the HTTP pipeline and return without waiting. In this mode, the call may return having written less than the 
            requested amount of data. It is the callers responsibility to handle this occurrence.
            @return Count of bytes written. Returns null if the WebSocket is closed.
           @throws ArgError for bad options
         */
        native function sendBlock(content, options): Number

        /**
            The URI provided to the constructor
         */
        native function get url(): Uri

        /** 
            Wait for the WebSocket to achieve a desired state. 
            @param state Desired ready state value
            @param timeout Timeout in milliseconds to wait for the desired state. A timeout of zero means don't block.
            A timeout of < 0 (default), means wait forever.
            @return True if the desired state was achieved
         */
        native function wait(state: Number, timeout: Number = -1): Boolean
    }
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
