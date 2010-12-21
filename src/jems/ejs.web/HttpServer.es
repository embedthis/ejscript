/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

module ejs.web {

    enumerable dynamic class HttpServer {
        use default namespace public

        /**
            Index files list. Index files are used by various handlers when requests to directories are made. The 
            indicies are tried in turn for the first valid index file.
          */
        static var indicies = ["index.ejs", "index.html"]

        /** 
            Get the local IP address bound to this socket.
            @returns A string containing the address in dot notation. Returns the empty string if listening on all
            interfaces and returns null if the server is not bound (listening) to any address.
         */
        native function get address(): String 

        /** 
            @duplicate Stream.async
            If the server is put into sync mode, it must be done before calling listen.
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            Server configuration. Initially refers to App.config which is filled with the aggregated "ejsrc" content.
            If a documentRoot/ejsrc exists, it is loaded once at startup and is blended (overwrites) existing 
            App.config settings.
         */
        enumerable var config: Object

        /** 
            Default local directory for web documents to serve. This is used as the default Request.dir value.
         */
        var documentRoot: Path

        /** 
            Flag indicating if the server is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native function get isSecure(): Boolean

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option clients Maximum number of simultaneous clients.
            @option headers Maximum number of headers in a request.
            @option header Maximum size of headers.
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requests Maximum number of simultaneous requests.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option reuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            @option sessions Maximum number of simultaneous sessions.
            @option sessionTimeout Maximum time to preserve session state. Set to zero for no timeout.
            @option stageBuffer Maximum stage buffer size for each direction.
            @option transmission Maximum size of outgoing body data.
            @option upload Maximum size of uploaded files.
            @option uri Maximum size of URIs.
            @see setLimits
          */
        native function get limits(): Object

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
            Default root directory for the server. The app does not change its current directory to this path.
         */
        var serverRoot: Path

        /** 
            Hash of session objects. This is created on demand as requests require session state storage.
         */
        var sessions: Object

        /** 
            Software details for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String

        /** 
            Create a HttpServer object. The server is created in async mode by default.
            If an "ejsrc" file exists in the server root, it will be loaded and update the "$config" properties.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
MOB - more explanation about what is in the ServerRoot
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server. The serverRoot directory may contain an optional "ejsrc"
                configuration file to load.
            @spec ejs
            @stability prototype
            @example: This is a fully async server:

            let server: HttpServer = new HttpServer(".", "web")
            let router = Router(Router.Restful)
            server.on("readable", function (event: String, request: Request) {
                request.status = 200
                request.setHeaders({"Content-Type": "text/plain"})
                request.on("readable", function (event, request) {
                    let data = new ByteArray
                    if (request.read(data) == null) {
                        print("EOF")
                    }
                })
                request.on("writable", function (event) {
                    request.write("Hello World")
                    request.finalize()
                })
            }
            server.listen("127.0.0.1:7777")
         */
        function HttpServer(documentRoot: Path = ".", serverRoot: Path = ".") {
            this.documentRoot = documentRoot
            this.serverRoot = serverRoot
            config = App.config
            if (serverRoot != ".") {
                let path = serverRoot.join("ejsrc")
                if (path.exists) {
                    blend(config, path.readJSON(), true)
                    App.updateLog()
                }
            }
            let dirs = config.directories
            for (let [key,value] in dirs) {
                dirs[key] = documentRoot.join(value)
            }
        }

        /** 
            Accept a new incoming for sync servers.  This call creates a request object in response to an 
            incoming client connection on the current HttpServer object.  In async mode, the accept() call is not needed
            as the HttpServer automatically creates the Request object and passes it on "readable" events.
            @return A Request object if in sync mode. No return value if in async mode. 
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new HttpServer
                server.listen("8080")
                while (request = server.accept()) {
                    Web.serve(request)
                }
         */
        native function accept(): Request

        /** 
            @duplicate Stream.close 
         */
        native function close(): Void

        /** 
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the endpoint argument is not required and is ignored.

            HttpServer supports both sync and async modes of operation.  In sync mode, after listen call is made, 
            $accept must be called to wait for and receive client connections. The $accept call will create the 
            Request object.  In async mode, Request objects will be created automatically and passed to registered 
            observers via "readable" events.

            @param endpoint The endpoint address on which to listen. An endoint may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.on("readable", function (event, request) {
                    //  NOTE: this is set to the request
                    Web.serve(request)
                })
                server.listen("80")
         */
        native function listen(endpoint: String? = null): Void

        /** 
            Remove an observer from the server. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function off(name: Object, observer: Function): Void

        /** 
            Add an observer for server events. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param observer Callback listening function. The function is called with the following signature:
                function on(event: String, ...args): Void
            @event readable Issued when there is a new request available. This readable event will explicitlyl set the
                value of "this" to the request regardless of whether the function has a bound "this" value.
            @event close Issued when server is being closed.
            @event createSession Issued when a new session store object is created for a client. The request object is
                passed.
            @event destroySession Issued when a session is destroyed. The request object is passed.
         */
        native function on(name, observer: Function): Void

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
            Define the stages of the Http processing pipeline. Data flows through the processing pipeline and is
            filtered or transmuted by filter stages. A communications connector is responsible for transmitting to 
            the network.
            @param incoming Array of stages for the incoming pipeline: default: ["chunk", "range", "upload"]
            @param outgoing Array of stages for the outgoing pipeline: default: ["auth", "range", "chunk"]
            @param connector Network connector to use for I/O. Defaults to the network connector "net". Other values: "send".
                The "net" connector transparently upgrades to the "send" connector if transmitting static data and 
                not using SSL, ranged or chunked transfers.
         */
        native function setPipeline(incoming: Array, outgoing: Array, connector: String): Void

        /**
            Update the server resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /**
            Configure request tracing for the server. The default is to trace the first line of requests and responses at
            level 2 and to trace headers at level 3. The options argument contains optional properties: rx and tx 
            (for receive and transmit tracing). The rx and tx properties may contain an object hash which describes 
            the tracing for that direction and includes any of the following fields:
            @param options. Set of trace options with properties "rx" and "tx" for receive and transmit direction tracing.
                The include property is an array of file extensions to include in tracing.
                The include property is an array of file extensions to exclude from tracing.
                The all property specifies that everything for this direction should be traced.
                The conn property specifies that new connections should be traced.
                The first property specifies that the first line of the request should be traced.
                The headers property specifies that the headers (including first line) of the request should be traced.
                The body property specifies that the body content of the request should be traced.
                The size property specifies a maximum body size in bytes that will be traced. Content beyond this limit 
                    will not be traced.
            @option transmit. Object hash with optional properties: include, exclude, first, headers, body, size.
            @option receive. Object hash with optional properties: include, exclude, conn, first, headers, body, size.
            @example:
                trace({
                    transmit: { exclude: ["gif", "png"], "headers": 3, "body": 4, size: 1000000 }
                    receive:  { "conn": 1, "headers": 2 , "body": 4, size: 1024 }
                })
          */
        native function trace(options: Object): Void

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
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
