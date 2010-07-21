/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

module ejs.web {

    enumerable dynamic class HttpServer {
        use default namespace public

        /** 
            Create a HttpServer object. The server is created in async mode by default.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server.
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
        native function HttpServer(documentRoot: Path = ".", serverRoot: Path = ".")

        /** 
            Accept a for client connection. This creates a request object in response to an incoming client connection
            on the current HttpServer object. This call is only required in sync mode. 
            In async mode, the HttpServer automatically creates the Request object and passes it on "readable" events.
            @return A Request object if in sync mode. No return value if in async mode. 
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.listen("80")
                while (request = server.accept()) {
                    Web.serve(request)
                }
         */
        native function accept(): Request

        /** 
            Get the local IP address bound to this socket.
            @returns the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

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

        static var indicies = ["index.ejs", "index.html"]

        /** 
            Flag indicating if the server is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native function get isSecure(): Boolean

        /** 
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the endpoint argument is ignored.

            HttpServer supports both sync and async modes of operation.  In sync mode, after listen call is made, 
            $accept must be called to wait for and receive client connections. The $accept call will create the 
            Request object.  In async mode, Request objects will be created automatically and passed to registered 
            observers via "readable" events.

            @param endpoint The endpoint address on which to listen. An endoint is a port number or a composite 
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
        native function listen(endpoint: String?): Void

        /**
            The authorized public host name for the server. If defined, this name will be used in preference for 
            request redirections. Defaults to the listening IP address if specified.
         */
        native function get name(): String 
        native function set name(hostname: String): Void

        /** 
            Get the port bound to this Http endpoint.
            @return The port number or 0 if it is not bound.
         */
        native function get port(): Number 

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
            Remove an observer from the server. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            Define the Secure Sockets Layer (SSL) protocol credentials. This must be done before calling $listen.
            @param keyFile Path of the file containing the server's private key. This file
            contains the PEM encoded private key file for the server. Set to null if the private key is combined with 
            the certificate file. If the private key is encrypted, you will be prompted at the console to enter the 
            pass-phrase to decript the private key on system reboot. There is a delima here. If you use a crypted 
            private key, the server will pause until you enter the pass-phrase which makes headless operation impossible. 
            If you do not encrypt the private key, your private key is more vulnerable should the server be compromised. 
            Which option you choose depends on whether headless operation is essential or not.
            @param certFile Path of the file containing the SSL certificate
            The certificate file contains the PEM encoded X.509 certificate for the server. The file may also contain 
            the private key in which case you should set the key parameter to null.
            The path may be an absolute path or it may be relative to the ServerRoot.
            @param protocols Optional arary of SSL protocols to support. Select from: SSLv2, SSLv3, TLSv1, ALL. 
                Each protocol can be prefixed by "+" or "-" to add or subtract from the prior set.
                For example: ["ALL", "-SSLv2"], or ["SSLv3", "TLSv1"] or "[ALL]"
            @param ciphers Optional array of ciphers to use when negotiating the SSL connection. Not yet supported.
            @throws ArgError for invalid arguments
         */
        native function secure(keyFile: Path, certFile: Path!, protocols: Array = null, ciphers: Array = null): Void

        /**
            Define resource limits for the server. Some of these limit values are also used for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunkSize Maximum size of a chunk when using chunked transfer encoding
            @option headerCount Maximum number of headers in a request
            @option headerSize Maximum size of headers
            @option receiveBodySize Maximum size of incoming body data
            @option stageBufferSize Maximum stage buffer size for each direction
            @option transmissionBodySize Maximum size of outgoing body data
            @option uploadSize Maximum size of uploaded files
            @option uriSize Maximum size of URLs
            @option clientCount Maximum number of simultaneous clients
            @option keepAliveCount Maximum number of times to reuse a connection for requests
            @option requestCount Maximum number of simultaneous requests
            @option sessionCount Maximum number of simultaneous sessions
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle
            @option requestTimeout Maximum time in seconds for a request to complete
            @option sessionTimeout Maximum time to preserve session state
          */
        native function setLimits(limits: Object): Void

        /**
            Return an object hash with the current server resource limits
          */
        native function getLimits(): Object

        /**
            Define the operation options for the server.
            @param options. Options is an object hash with the following properties:
            @option directoryListings
          */
        native function setOptions(options: Object)

        /**
            Return an object hash with the current server options
          */
        native function getOptions(): Object

        /**
            Configure request tracing for the server
            @param level Level at which request tracing will occurr
            @param size Maximum request body size to trace
          */
        native function trace(level: Number, options: Object = ["headers", "request", "response"], size: Number = null): Void

        /**
            Configure trace filters for request tracing
          */
        native function traceFilter(include: Array = ["*"], exclude: Array = ["gif", "ico", "jpg", "png"]): Void

        /**
            @param incoming Array of stages for the incoming pipeline: default: ["chunk", "range", "upload"]
            @param outgoing Array of stages for the outgoing pipeline: default: ["auth", "range", "chunk"]
            @param connector Network connector to use for I/O. Defaults to the network connector "net". This connector
                transparently upgrades to the sendfile connector if transmitting static data and not using SSL, ranged or 
                chunked transfers.
         */
        native function setPipeline(incoming: Array = ["chunk", "range", "upload"], 
                outgoing: Array = ["auth", "range", "chunk"], connector: String = "net"): Void

        /**
            Verify client certificates. This ensures that the clients must provide a client certificate for to verify 
            the their identity. You can choose to use either the caCertPath or caCertFile argument. If both are provided
            caCertPath takes precedence.
            @param caCertPath Defines the directory containing the certificates to use for client authentication.
            The path may be an absolute path or it may be relative to the ServerRoot.
            Set to null if you are using $caCertFile.
            @param caCertFile Defines the location of the certificate file or bundle to use for client authentication.
                Use this if you have a single certificate or a bundle of certificates.
                Set to null if you are using $caCertPath.
         */
        native function verifyClients(caCertPath: Path, caCertFile: Path): Void

        /** 
            Default root directory for the server. The app does not change its current directory to this path.
         */
        var serverRoot: Path

        /** 
            Software details for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String
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
