/*
    Web.es - Web framework initialization
    This loads JSGI (*.es) apps, Template (*.ejs) pages, and MVC applications.
 */

module ejs.web {
    /** 
        Web class manages web applications. This class initializes the web framework and loads web applications. 
        Apps may be JSGI apps with a *.es extension, template apps with a ".ejs" extension or MVC applications.
        @spec ejs
        @stability prototype
     */
    class Web {
        use default namespace public

        // static var config

        private static var defaultConfig = {
            cache: {
                enable: true,
                //  MOB - not yet supported
                reload: false,
            },
            directories: {
                cache: Path("cache"),
                layouts: Path("layouts"),
                views: Path("views"),
            },
            extensions: {
                es:  "es",
                ejs: "ejs",
                mod: "mod",
            },
            log: {
                showClient: true,
                //  where: "file" - defaults to web server log
            },
            // MOB -- not yet implemented
            session: {
                //  MOB -- do we need enable
                enable: true,
                //  MOB -- is this being used?
                timeout: 1800,
            },
            web: {
                expires: {
                    /*
                        MOB -- can we have some of this be the default?
                        html:   86400,
                        ejs:    86400,
                        es:     86400,
                        "":     86400,
                     */
                },
                // endpoint: "127.0.0.1:4000",
                // helpers: [],
                view: {
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

        /*  
            One time initialization for the Web class. Loads the top-level "ejsrc" configuration file.
            The server must be restarted to reload changes. This happens before HttpServer loads serverRoot/ejsrc.
         */
        private static function initWeb(): Void {
        /*
            MOB - remove
            config = App.config
            let path = Path("ejsrc")
            if (path.exists) {
                let webConfig = deserialize(path.readString())
                blend(config, webConfig, true)
            }
        */
            blend(App.config, defaultConfig, false)
        }
        initWeb()

        /** 
            Serve a web request. Convenience function to route, load and start a web application. 
            Called by web application start script
            @param request Request object
            @param router Configured router instance. If omitted, a default Router will be created using the Router.Top
                routing table.
         */
        static function serve(request: Request, router: Router = Router()): Void {
            try {
                let app = router.route(request)
                if (request.route.threaded) {
                    worker(app, request)
                } else {
                    process(app, request)
                }
            } catch (e) {
                let status = request.status != Http.Ok ? request.status : Http.ServerError
                request.writeError(status, e)
            }
        }

        /**
            Run the request via a separate worker thread
            @param app Application function to run
            @param request Request object
         */
        static native function worker(app: Function, request: Request): Void

        private static function workerHelper(app: Function, request: Request): Void {
            try {
                process(app, request)
            } catch (e) {
                request.writeError(Http.ServerError, e)
            }
        }

        //  MOB -- where here should content mapping take place according to Accept: 
        //    Accept: application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5
        private static function processBody(request: Request, body: Object): Void {
            if (body is Path) {
                //  MOB -- should have generic way of disabling writeFile
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
//  MOB Must detect eof and do a finalize()
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

//  MOB WARNING: this may block in write?? - is request in async mode?
//  MOB -- is this the best name?
        /** 
            Process a web request
            @param request Request object
            @param app Web application function 
         */
        static function process(app: Function, request: Request, finalize: Boolean = true): Void {
            request.config = request.server.config
            try {
                if (request.route && request.route.middleware) {
                    app = Middleware(app, request.route.middleware)
                }
                if (finalize) {
                    request.setupFlash()
                }
                let response
                if (app.bound != global) {
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
                    processBody(request, response.body)
                } else {
                    let file = request.responseHeaders["X-Sendfile"]
                    if (file && !request.isSecure) {
                        request.writeFile(file)
                    }
                }
                if (finalize) {
                    request.finalizeFlash()
                    request.autoFinalize()
                }
            } catch (e) {
                App.log.debug(3, e)
                request.writeError(Http.ServerError, e)
            }
        }

        /**
            Convenience routine to start a routing web server that will serve a variety of content. This routines
            sets up a web server using the specified route tables.
            @param address The IP endpoint address on which to listen. The address may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server.
            @param routes Route table to use. Defaults to Router.Top
         */
        static function start(address: String, documentRoot: Path = ".", serverRoot: Path = ".", 
                routes = Router.Top): Void {
            let server: HttpServer = new HttpServer(documentRoot, serverRoot)
            var router = Router(routes)
            server.on("readable", function (event, request) {
                serve(request, router)
            })
            server.listen(address)
            App.eventLoop()
        }

        /**
            Convenience routine to run a single web app script. 
            @param address The IP endpoint address on which to listen. The address may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server.
            @example The script must be of the form:
            exports.app = function (request) {
                return { 
                    status: Http.Ok,
                    body: "Hello World\r\n"
                }
            }
         */
        static function run(address: String, documentRoot: Path = ".", serverRoot: Path = "."): Void {
            let server: HttpServer = new HttpServer(documentRoot, serverRoot)
            server.on("readable", function (event, request) {
                try {
                    if (!request.filename.exists) {
                        request.writeError(Http.NotFound, "Cannot find " + request.uri)
                    } else {
                        process(Loader.require(request.filename).app)
                    }
                } catch {
                    request.writeError(Http.ServerError, "Exception serving " + request.uri)
                }
            })
            server.listen(address)
            App.eventLoop()
        }
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
