/**
    WebSocket.es -- WebSockets class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        WebSocket compatible method to retrieve HTTP data
        This code is prototype and is not yet supported
        @spec ejs
        @hide
        @stability evolving
     */
    class WebSocket implements Stream {

        use default namespace public

        /** readyState value */
        static const CONNECTING = 0              

        /** readyState value */
        static const OPEN = 1

        /** readyState value */
        static const CLOSING = 2

        /** readyState value */
        static const CLOSED = 3

        /**
            Create a new WebSocket and connect to the server using the supplied URI.
            @param uri URL to connect to
            @param protocols Optional password if authentication is required.
         */
        native function WebSocket(uri: Uri, protocols: String? = null)

        function get onopen(): Function
            null
        function set onopen(observer: Function): Void
            on("headers", observer)

        function get onmessage(): Function
            null
        function set onmessage(observer: Function): Void
            on("readable", observer)

        function get onerror(): Function
            null
        function set onerror(observer: Function): Void
            on("error", observer)

        function get onclose(): Function
            null
        function set onclose(observer: Function): Void
            on("close", observer)

        /**
            Type of binary data: "blob" or "arraybuffer"
          */
        native function get binaryType(): String

        /**
            The number of bytes of buffered send data
          */
        native function get bufferedAmount(): Number

        native function close(code: Number, reason: String? = "")

        /**
            Return the list of supported extensions
          */
        native function get extensions() : String

        /** 
            @duplicate Stream.off
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
         */
        native function on(name, observer: Function): Http

        /**
            Return the selected protocol
          */
        native function get protocol() : String

        /**
            The readystate value. This value can be compared with the WebSocket constants: Uninitialized, Open, Sent,
            Receiving or Loaded Set to: CONNECTING = 0, OPEN = 1, CLOSING = 2, CLOSED
         */
        native function get readyState() : Number

        /**
            Send data with the request.
            @param content Data to send with the request.
         */
        native function send(...content): Void
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
