/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

module ejs.web {

    /**
        HttpServer objects represents the server-side of Hypertext Transfer Protocol (HTTP) version 1.1 connections. 
        The HttpServer class is used to receive HTTP requests and generate responses. It supports the HTTP/1.1 standard
        including methods for GET, POST, PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections.
        @spec ejs
        @stability evolving
     */
    enumerable dynamic class HttpServer {
        use default namespace public

        /** Frequency to check and release excess worker threads */
        static var PrunePeriod = 60 * 1000

        private var idleWorkers: Array = []
        private var activeWorkers: Array = []
        private var workerImage: Worker

        private static const defaultConfig = {
            dirs: {
                cache: Path("cache"),
                layouts: Path("layouts"),
            },
            extensions: {
                es:  "es",
                ejs: "ejs",
                mod: "mod",
            },
            log: {
                showErrors: true,
            },
            cache: {
                adapter: "local",
                "class": "LocalCache",
                module: "ejs",
                lifespan: 3600,
                app:     { enable: true, reload: true },
                actions: { enable: true },
                records: { enable: true },
                workers: { enable: true, limit: 10 },
            },
            web: {
                limits: {},
                views: {
                    connectors: {
                        table: "html",
                        chart: "google",
                        rest: "html",
                    },
                    formats: {
                        Date: "%a %e %b %H:%M",
                    },
                    layout: "default.ejs",
                },
            },
        }

        /**
            One time initialization. Blend mandatory config into App.config.
            @hide
         */
        static function initHttpServer() {
            blend(App.config, defaultConfig, {overwrite: false})
            let dirs = App.config.dirs
            for (let [key, value] in dirs) {
                dirs[key] = Path(value)
            }
            if (!App.cache) {
                App.cache = new Cache(null, blend({shared: true}, App.config.cache))
            }
        }
        initHttpServer()

        /**
            Index files list. Index files are used by various handlers when requests to directories are made. The 
            indicies are tried in turn for the first valid index file.
          */
        static var indicies: Array = ["index.ejs", "index.html"]

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
            If a "ejsrc" configuration file exists, it is loaded once at startup and is blended (overwrites) existing 
            App.config settings.
         */
        enumerable var config: Object

        /** 
            Default local directory for web documents to serve. This is used as the default Request.dir value.
         */
        var documents: Path

        /**
            @hide
         */
        native function get hostedHome(): Path

        /** 
            Flag indicating if the server is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native function get isSecure(): Boolean

        /**
            Flag indicating if the server is running hosted inside a web server
         */
        native function get hosted(): Boolean
        native function set hosted(value: Boolean): Void

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option clients Maximum number of simultaneous clients.
            @option connReuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            @option headers Maximum number of headers in a request.
            @option header Maximum size of headers.
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requests Maximum number of simultaneous requests.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option sessionTimeout Default time to preserve session state for new requests. Set to zero for no timeout.
            @option stageBuffer Maximum stage buffer size for each direction.
            @option transmission Maximum size of outgoing body data.
            @option upload Maximum size of uploaded files.
            @option uri Maximum size of URIs.
            @option workers Maximum number of Worker threads to utilize for threaded requests. This value is initialized
                from the ejsrc cache.workers.limit field.
            @see setLimits
          */
        native function get limits(): Object

        /**
            The authorized public host name for the server. If defined, this name will be used in preference for 
            request redirections. If no name is defined, redirections will use to the listening IP address by default.
         */
        native function get name(): String 
        native function set name(hostname: String): Void

        /**
            HttpServer constructor options
         */
        var options: Boolean

        /** 
            Get the port bound to this Http endpoint.
            @return The port number or 0 if it is not bound.
         */
        native function get port(): Number 

        /**
            Function to invoke inside a worker when serving a threaded request. The function is invoked with the signature:
            function onrequest(request: Request): Void
         */
        public var onrequest = defaultOnRequest

        /*
            Default onrequest function to handle threaded requests
            @param request Request object
         */
        private function defaultOnRequest(request: Request): Void {
            App.log.debug(6, "Multithreaded request")
            try {
                process(request.route.response, request)
            } catch (e) {
                request.writeError(Http.ServerError, e)
            }
        }

        /** 
            Default root directory for the server. The app does not change its current directory to this path.
         */
        var home: Path

        /** 
            Software description for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String

        /** 
            Create a HttpServer object. The server is set to async mode by default.
            If an "ejsrc" file exists in the server root, it will be loaded.
            @param options. Set of options to configure the server.
            @option documents Directory containing web documents to serve. If unset and the HttpServer is hosted,
                the $documents property will be defined by the web server.
            @option home Base directory for the server and the "ejsrc" configuration file. If unset and the HttpServer is 
                hosted, the $home property will be defined by the web server.
            @option ejsrc Alternate path to the "ejsrc" configuration file
            @option config Alternate App.config settings
            @option unhosted If hosted inside a web server, set to true to bypass any web server listening endpoints and 
                create a new stand-alone (unhosted) listening connection.
            @spec ejs
            @stability prototype
            @example: This is a fully async server:
let server: HttpServer = new HttpServer({documents: "web"})
let router = Router(Router.Restful)
server.on("readable", function (event: String, request: Request) {
    // "this" object is also set to request
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
        function HttpServer(options: Object = {}) {
            if (options.unhosted) {
                hosted = false
            }
            if (hosted) {
                let path = hostedHome
                documents = options.documents || path
                home = options.home || path
            } else {
                documents = options.documents || "."
                home = options.home || "."
            }
            home = home.relative
            config = options.config || App.config
            this.options = options
            let ejsrc = options.ejsrc || config.files.ejsrc
            if (ejsrc.exists && !App.config.files.ejsrc.same(ejsrc)) {
                blend(config, Path(config.files.ejsrc).readJSON())
                App.updateLog()
            } else if (home != ".") {
                let path = home.join("ejsrc")
                if (path.exists) {
                    blend(config, path.readJSON())
                    App.updateLog()
                }
            }
            /* Prefix dirs with the home directory */
            let dirs = config.dirs
            for (let [key, value] in dirs) {
                dirs[key] = home.join(value)
            }
            let web = config.web
            if (web.trace) {
                trace(web.trace)
            }
            web.limits.workers ||= config.cache.workers.limit
            setLimits(web.limits)
            if (web.session) {
                openSession()
            }
            //  MOB - BUG. Need this.fun to bind the function
            setInterval(this.pruneWorkers, PrunePeriod, this)
        }

        private function openSession() {
            let sconfig = config.session
            let sclass = sconfig["class"]
            if (sclass) {
                if (sconfig.module && !global.(module)::[sclass]) {
                    global.load(sconfig.module + ".mod", {reload: false})
                }
                let module = sconfig.module || "public"
                new (module)::[sclass](sconfig.adapter, sconfig.options)
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
                    server.serve(request)
                }
         */
        native function accept(): Request

        /** 
            @duplicate Stream.close 
         */
        native function close(): Void

        /*
            Get a worker for a multithreaded request. This will clone a worker from the workerImage and will
            enforce the configured limits.workers value.
         */
        private function getWorker(): Worker {
            let w = idleWorkers.pop()
            if (w == undefined) {
                if (limits.workers && activeWorkers.length >= limits.workers) {
                    App.log.debug(1, "Too many workers " + activeWorkers.length + "/" + limits.workers)
                    return null
                }
                if (workerImage == null) {
                    workerImage = new Worker
                    workerImage.preeval("require ejs.web")
                }
                w = workerImage.clone()
                if (config.web.workers && config.web.workers.init) {
                    w.preeval(config.web.workers.init)
                }
            }
            activeWorkers.push(w)
            App.log.debug(4, "HttpServer.getWorker idle: " + idleWorkers.length + " active:" + activeWorkers.length)
            return w
        }

        /** 
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When hosted inside a web server, the web server will define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen() is called, the HttpServer object will be bound to
            the actual web server's listening connection. In this case, the endpoint argument is not required and 
            is ignored.

            HttpServer supports both sync and async modes of operation.  In sync mode, after listen call is made, 
            $accept must be called to wait for and receive client connections. The $accept call will create the 
            Request object.  In async mode, Request objects will be created automatically and passed to registered 
            observers via "readable" events.

            @param endpoint The endpoint address on which to listen. An endoint may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            The IP portion may also be "*" to indicate all interfaces.
            If the server is hosted in a web server, an appropriate existing web server listening connection will be 
            used. Otherwise, if not hosted, a server socket will be opened on the endpoint.
            If hosted and an endpoint is not provided, the server will listen on all appropriate web server connections.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http({home: ".", documents: "web"})
                server.on("readable", function (event, request) {
                    //  NOTE: "this" is set to the request
                    server.serve(request)
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
            @event readable Issued when there is a new request available. This readable event will explicitly set the
                value of "this" to the request regardless of whether the function has a bound "this" value.
            @event close Issued when server is being closed.
         */
        native function on(name, observer: Function): Void

        /**
            Pass a request into a worker VM. The onrequest callback receives the request. This routine clones stub 
            Request and HttpServer objects into the worker VM.
            @param request Request object
            @param worker Worker to handle the request
         */
        native function passRequest(request: Request, worker: Worker)

        /** 
            Process a web request
            @param request Request object
            @param app Web application function 
         */
        function process(app: Function, request: Request, finalize: Boolean = true): Void {
// let mark = new Date
            request.config = config
            try {
                if (request.route && request.route.middleware) {
                    app = Middleware(app, request.route.middleware)
                }
                if (finalize) {
                    request.setupFlash()
                }
                let response
                if (app.bound) {
                    response = app(request)
                } else {
                    response = app.call(request, request)
                }
                if (response is Function) {
                    /* Functions don't auto finalize. The user is responsible for calling finalize() */
                    response.call(request, request)

                } else if (response) {
                    request.status = response.status || 200
                    let headers = response.headers || { "Content-Type": "text/html" }
                    request.setHeaders(headers)
                    if (response.body) {
                        processBody(request, response.body)
                    }

                } else {
                    let file = request.responseHeaders["X-Sendfile"]
                    if (file && !request.isSecure && !request.finalized) {
                        request.writeFile(file)
                    }
                }
                if (finalize) {
                    request.finalizeFlash()
                    request.autoFinalize()
                }
            } catch (e) {
                App.log.debug(1, e)
                request.writeError(Http.ServerError, e)
            }
// App.log.debug(2, "LEAVE PROCESSING  " + mark.elapsed + " msec for " + request.uri)
        }

        private function processBody(request: Request, body: Object): Void {
            if (body is Path) {
                if (request.isSecure) {
                    body = File(body, "r")
                } else {
                    request.writeFile(body)
                    return
                }
            }
            if (body is Array) {
                for each (let item in body) {
//  MOB -- what about async? what if can't accept all the data?
                    request.write(item)
                }
                request.autoFinalize()

            } else if (body is Stream) {
                if (body.async) {
                    request.async = true
                    //  Should we wait on request being writable or on the body stream being readable?
//  MOB Must detect EOF and do a finalize()
                    request.on("readable", function(event, request) {
                        let data = new ByteArray
                        if (request.read(data)) {
//  MOB -- what about async? what if can't accept all the data?
                            request.write(body)
                        } else {
                            request.autoFinalize()
                        }
                    })
                    //  MOB -- or this? but what about error events
                    request.on("complete", function(event, body) {
                        request.autoFinalize()
                    })
                } else {
                    ba = new ByteArray
                    while (body.read(ba)) {
//  MOB -- exceptions on all these writes should be caught --- normal situation for client to disappear
                        request.write(ba)
                    }
                    request.autoFinalize()
                }
            } else if (body && body.forEach) {
                body.forEach(function(block) {
                    request.write(block)
                })
                request.autoFinalize()

            } else if (body is Function) {
                /* Functions don't auto finalize. The user is responsible for calling finalize() */
                body.call(request, request)

            } else if (body) {
                request.write(body)
                request.autoFinalize()

            } else {
                let file = request.responseHeaders["X-Sendfile"]
                if (file && !request.isSecure) {
                    request.writeFile(file)
                } else {
                    request.autoFinalize()
                }
            }
        }

        /** 
            Prune idle worker threads. This will release cached worker interpreters and reduce memory footprint. 
            After calling, the next request will be a little slower as it will need to recreate a worker interpreter.
            This is normally run every PrunePeriod. It may be also be called manually at any time.
         */
        function pruneWorkers(): Void {
            if (idleWorkers.length > 0) {
                App.log.debug(6, "HttpServer prune " + idleWorkers.length + " workers")
                idleWorkers = []
                GC.run()
            }
        }

        private function releaseWorker(w: Worker): Void {
            activeWorkers.remove(w)
            if (config.cache.workers.enable) {
                idleWorkers.push(w)
            }
            App.log.debug(4, "HttpServer.releaseWorker idle: " + idleWorkers.length + " active:" + activeWorkers.length)
        }

        /** 
            Run the application event loop to service requests.
            If the HttpServer is hosted in a web server, this call does nothing as the web server will service events and
            will return immediately. 
         */
        native function run(): Void

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
            Serve a web request. Convenience function to route, load and start a web application. 
            Called by web application start script
            @param request Request object
            @param router Configured router instance. If omitted, a default Router will be created using the Router.Top
                routing table.
         */
        function serve(request: Request, router: Router = Router()): Void {
            request.mark = new Date
            try {
                let w: Worker
                let route: Route = router.route(request)
                if (route.threaded) {
                    if ((w = getWorker()) == null) {
                        request.writeError(Http.ServiceUnavailable, "Server busy")
                        return
                    }
                    request.on("close", function() {
                        releaseWorker(w) 
                        App.log.debug(3, "Elapsed " + request.mark.elapsed + " msec for " + request.uri)
                    })
                    passRequest(request, w)
                    /* Must not touch request from here on - the worker owns it now */
                } else {
                    //  MOB - rename response => responder
                    let mark = new Date
                    process(route.response, request)
                    App.log.debug(3, "Elapsed " + mark.elapsed + " msec for " + request.uri)
                }
            } catch (e) {
                let status = request.status != Http.Ok ? request.status : Http.ServerError
                request.writeError(status, e)
            }
        }

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
            Define a worker that will serve as the base for cloning workers to serve web requests
            @param worker Configured worker 
         */
        function setWorkerImage(worker: Worker): Void
            workerImage = worker

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

        /**
            Convenience routine to create a web server. This will start a routing web server that will serve a 
            variety of content using the given specified route tables.
            @param address The IP endpoint address on which to listen. The address may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param options. Set of options to configure the server.
            @option documents Directory containing web documents to serve. If unset and the HttpServer is hosted,
                the $documents property will be defined by the web server.
            @option home Base directory for the server and the "ejsrc" configuration file. If unset and the HttpServer is 
                hosted, the $home property will be defined by the web server.
            @option routes Route table to use. Defaults to Router.Top
         */
        static function create(address: String, options: Object = {}): Void {
            let server: HttpServer = new HttpServer(options)
            let routes = options.routes || Router.Top
            var router = Router(routes)
            server.on("readable", function (event, request) {
                server.serve(request, router)
            })
            server.listen(address)
            App.run()
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
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
