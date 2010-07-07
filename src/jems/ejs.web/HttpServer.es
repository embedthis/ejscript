/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

module ejs.web {

    dynamic class HttpServer {
        use default namespace public

//  MOB -- remove serverRoot completely
        /** 
            Create a HttpServer object. The server is created in async mode by default.
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
            @spec ejs
            @stability prototype
            @example: This is a fully async server:

            let server: HttpServer = new HttpServer(".", "web")
            let router = Router(Router.RestfulRoutes)
            server.observe("readable", function (event: String, request: Request) {
                request.status = 200
                request.setHeaders({"Content-Type": "text/plain"})
                request.observe("readable", function (event, request) {
                    let data = new ByteArray
                    if (request.read(data) == null) {
                        print("EOF")
                    }
                })
                request.observe("writable", function (event) {
                    request.write("Hello World")
                    request.finalize()
                })
            }
            server.listen("127.0.0.1:7777")
         */
        native function HttpServer(serverRoot: Path = ".", documentRoot: Path = ".")

        /** 
            Add an observer for server events. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param observer Callback listening function. The function is called with the following signature:
                function observer(event: String, ...args): Void
            @event readable Issued when there is a new request available
            @event close Issued when server is being closed.
            @event createSession Issued when a new session store object is created for a client. The request object is
                passed.
            @event destroySession Issued when a session is destroyed. The request object is passed.
         */
        native function observe(name, observer: Function): Void

        /** 
            Get the local IP address bound to this socket.
            @returns the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

//  MOB - does sync mode make any sense? Maybe in threaded-appweb?
        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async
         */
        native function set async(enable: Boolean): Void

        /** 
            @duplicate Stream.close */
        native function close(): Void

        /** 
            Default local directory for web documents to serve. This is used as the default Request.dir value.
         */
        var documentRoot: Path

//  MOB - how to do SSL?
//  MOB -- not right for sync mode. Never returns a request
        /** 
            Listen for client connections. This creates a listening HTTP server. 
            If the server is in sync mode, the listen call will block until a client connection is received and
            the call will return a request object.
            If a the socket is in async mode, the listen call will return immediately and client connections
            will be notified via "accept" events. In this case when a client connection is received, the $accept function 
            must be called to receive the Request object.
            @return A Request object if in sync mode. No return value if in async mode. 
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @throws ArgError if the specified listen address is not valid, and IOError for network errors.
            @event Issues a "accept" event when there is a new connection available. In response, the $accept method
                should be called.
         */
        native function listen(address): Request

//  MOB
        native function attach(): Void

// MOB
        /** @hide */
        native function secureListen(address, keyFile, certFile, protocols: String, ciphers: String): Void

        /** 
            Get the port bound to this Http endpoint.
            @return The port number or 0 if it is not bound.
         */
        native function get port(): Number 

        /** 
            Remove an observer from the server. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            Default root directory for the server. The app does not change its current directory to this path.
            MOB -- is this needed?
         */
        var serverRoot: Path

        /** 
            Software details for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String

        /** 
            Get the count of active sessions
            @return The number of active sessionss
         */
        native static function get sessionCount(): Number
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
