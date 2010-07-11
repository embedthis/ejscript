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
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the endpoint argument is ignored.

            HttpServer supports both sync and async modes of operation. If the server is in sync mode, the listen call 
            will block until a client connection is received and the call will return a request object. If a the socket 
            is in async mode, the listen call will return immediately and client connections will be notified via 
            "accept" events. 

            @return A Request object if in sync mode. No return value if in async mode. 
            @param address The endpoint address on which to listen. An endoint is a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.observe("readable", function (event, request) {
                    Web.serve(request)
                })
                server.listen("80")
         */
        native function listen(endpoint: String?): Request

// MOB
        /** @hide */
        /** 
            Listen for client connections using the Secure Sockets Layer (SSL)protocol. This creates a HTTP server 
            listening on a single socket endpoint for SSL connections. It can also be used to attach to an existing 
            listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the listen arguments are ignored.

            HttpServer supports both sync and async modes of operation. If the server is in sync mode, the secureListen call 
            will block until a client connection is received and the call will return a request object. If a the socket 
            is in async mode, the secureListen call will return immediately and client connections will be notified via 
            "accept" events. 

            @return A Request object if in sync mode. No return value if in async mode. 
            @param address The endpoint address on which to listen. An endoint is a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param keyFile Path of the file containing the server's private key
            @param certFile Path of the file containing the server's SSL certificate
            @param protocols Arary of SSL protocols to support. Select from: SSLv2, SSLv3, TLSv1, ALL. For example:
                ["SSLv3", "TLSv1"] or "[ALL]"
            @param ciphers Array of ciphers to use when negotiating the SSL connection. Not yet supported.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.observe("readable", function (event, request) {
                    Web.serve(request)
                })
                server.secureListen("443")
         */
        native function secureListen(endpoint: String?, keyFile: Path, certFile: Path, protocols: Array, 
            ciphers: Array): Void

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
