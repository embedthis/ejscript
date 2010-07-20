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

        static var config

        private static var defaultConfig = {
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
                        "html": 86400,
                        "ejs": 86400,
                        "es": 86400,
                        "": 86400,
                     */
                },
                endpoint: "127.0.0.1:4000",
                views: {
                    connectors: {
                        table: "html",
                        chart: "google",
                        rest: "html",
                    },
                    formats: {
                        currency:   "$%10f",
                        Date:       "%a %e %b %H:%M",
                    },
                },
            },
        }

        /*  
            One time initialization for the Web class. Loads the top-level "ejsrc" configuration file.
            The server must be restarted to reload changes.
         */
        private static function init(): Void {
            let path = Path("ejsrc")
            config = App.config
            if (path.exists) {
                let webConfig = deserialize(path.readString())
                blend(config, webConfig, true)
            }
            blend(config, defaultConfig, false)
        }
        init()

        /** 
            Serve a web request. Convenience function to route, load and start a web application. 
            Called by web application start script
            @param request Request object
            @param router Configured router instance. If omitted, a default Router will be created using the TopRoutes
                routing table.
         */
        static function serve(request: Request, router: Router = Router(Router.TopRoutes)): Void {
            try {
//  MOB -- should the router be setting this?
request.config = config
                router.route(request)
                if (request.route.threaded) {
                    worker(request)
                } else {
                    let exports = load(request)
                    if (exports) {
                        start(request, exports.app)
                    }
                }
            } catch (e) {
                request.writeError(e)
            }
        }

        /**
            Run the request via a separate worker thread
            @param request Request object
         */
        static native function worker(request: Request): Void

        private static function workerHelper(request: Request): Void {
            try {
                let exports = load(request)
                if (exports) {
                    start(request, exports.app)
                }
            } catch (e) {
                request.writeError(e)
            }
        }

        /** 
            Load a web app. This routine will load JSGI apps with a ".es" extension, template apps with a ".ejs" extension
            and MVC applications. This call expects that the request has been routed and that request.route.type is 
            set to the type of the request (es|ejs|mvc).
            @param request Request object
            @returns An exports object with an "app" property representing the application.
         */
        static function load(request: Request): Object {
            try {
                let exports
                let type = request.route.type

                let path = request.dir.join(request.pathInfo.trimStart('/'))
                if (path.isDir) {
                    exports = Dir.load(request)
                    if (exports) {
                        return exports
                    }
                    type = request.route.type
                }
                if (type == "static") {
                    return Static.load(request)

                } else if (type == "es") {
                    if (!global.Loader) {
                        global.load("ejs.cjs.mod")
                    }
                    let path = request.dir.join(request.pathInfo.slice(1))
                    if (!path.exists) {
                        throw "Request resource \"" + path + "\" does not exist"
                    }
                    exports = Loader.require(path)

                } else if (type == "ejs") {
                    if (!global.Template) {
                        global.load("ejs.web.template.mod")
                    }
                    let path = request.dir.join(request.pathInfo.slice(1))
                    if (!path.exists) {
                        request.writeError("Can't find \"" + path + "\".", Http.NotFound)
                        return null;
                    } else {
                        exports = Template.load(request)
                        request.setHeader("Content-Type", "text/html")
                    }

                } else if (type == "mvc") {
                    exports = Mvc.load(request)

                } else {
                    throw "Request type: " + type + " is not supported by Web.load"
                }
                if (!exports || !exports.app) {
                    throw "Can't load application. No \"app\" object exported by application"
                }
                return exports

            } catch (e) {
                request.writeError(e)
            }
            return null
        }


        private var clients = {}
        private var requestCount: Number

        function monitoredLoad(request: Request) {
            if (Object.getOwnPropertyCount(clients) >= HttpServer.limit("clients")) {
                request.writeError("Too many requests. Please try again later")
                return
            }
            if (requestCount >= HttpServer.limit("requests")) {
                request.writeError("Too many requests. Please try again later")
                return
            }
            requestCount++
            try {
                clients[request.remoteAddress] = true
                Object.getOwnPropertyCount(clients)
                delete clients[request.remoteAddress]
            } catch {}
            requestCount--
        }

        /** 
            Start running a loaded application.
            @param request Request object
            @param app Application function exported by the JSGI slice
         */
        static function start(request: Request, app: Function): Void {
//  WARNING: this may block in write?? - is request in async mode?
            try {
                let result = app.call(request, request)

                if (result is Function) {
                    /* Functions don't auto finalize. The user is responsible for calling finalize() */
                    result.call(request, request)

                } else if (result) {
                    let body = result.body
                    request.status = result.status || 200
                    let headers = result.headers || { "Content-Type": "text/html" }
                    request.setHeaders(headers)
                    if (body is Path) {
                        if (request.isSecure) {
                            body = File(body, "r")
                        } else {
                            request.sendfile(body)
                            return
                        }
                    }

                    if (body is Array) {
                        for each (let item in body) {
//  MOB -- what about async? what if can't accept all the data?
                            request.write(item)
                        }
                        request.finalize()

                    } else if (body is Stream) {
                        if (body.async) {
                            request.async = true
                            //  Should we wait on request being writable or on the body stream being readable?
//  MOB Must detect eof and do a finalize()
                            request.observe("readable", function(event, request) {
                                let data = new ByteArray
                                if (request.read(data)) {
//  MOB -- what about async? what if can't accept all the data?
                                    request.write(body)
                                } else {
                                    request.finalize()
                                }
                            })
                            //  MOB -- or this? but what about error events
                            request.observe("complete", function(event, body) {
                                request.finalize()
                            })
                        } else {
                            ba = new ByteArray
                            while (body.read(ba)) {
                                request.write(ba)
                            }
                            request.finalize()
                        }
                    } else if (body && body.forEach) {
                        body.forEach(function(block) {
                            request.write(block)
                        })
                        request.finalize()

                    } else if (body) {
                        request.write(body)
                        request.finalize()

                    } else {
                        let file = request.responseHeaders["X-Sendfile"]
                        if (file && !request.isSecure) {
                            request.sendfile(file)
                        } else {
                            request.finalize()
                        }
                    }
                } else {
                    let file = request.responseHeaders["X-Sendfile"]
                    if (file && !request.isSecure) {
                        request.sendfile(file)
                    }
                }

            } catch (e) {
                // print("URI " + request.uri)
                request.writeError(e)
                request.finalize(true)
            }
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
    
    @end
 */
