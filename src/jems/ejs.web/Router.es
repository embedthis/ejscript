/**
    Router.es - Web Request router. Route incoming client HTTP requests.
 */

module ejs.web {

    /** 
        Web router class. Routes incoming client HTTP requests to the appropriate location. The Route class supports 
        configurable user-defined routes. 

        Each application should create a Router instance and then attach matching routes.
        @example:

        var r = new Router

        //  Match files with a ".es" extension and use the ScriptBuilder to run
        r.add("es", /\.es$/,  {run: ScriptBuilder})

        //  Match directories and use the DirBuilder to run
        r.add(Router.isDir,    {name: "dir", run: DirBuilder})

        //  Match RESTful routes and run with the MvcBuilder
        r.add("new", "/:controller/new",        {run: MvcBuilder,    method: "GET",  action: "new"})
        r.add("edit", "/:controller/:id/edit",  {run: MvcBuilder,    method: "GET",  action: "edit"})
        r.add("show", "/:controller/:id",       {run: MvcBuilder,    method: "GET",  action: "show"})
        r.add("update", "/:controller/:id",     {run: MvcBuilder,    method: "PUT",  action: "update"})
        r.add("delete", "/:controller/:id",     {run: MvcBuilder,    method: "DELETE", action: "delete"})
        r.add("default", "/:controller/:action",{run: MvcBuilder})
        r.add("create", "/:controller",         {run: MvcBuilder,    method: "POST", action: "create"})
        r.add("index", "/:controller",          {run: MvcBuilder,    method: "GET",  action: "index"})
        
        //  Match an "admin" application. This sets the scriptName to "admin" expects an MVC application to be
        //  located at the directory "myApp"
        r.add("adminApp", "/admin/",        {location: { prefix: "/control", dir: "my"})

        //  Rewrite a request for "old.html" to new.html
        r.add("old", "/web/old.html",  {rewrite: function(request) { request.pathInfo = "/web/new.html"}})

        //  Handle a request with a literal response
        r.add("unknown", "/oldStuff/", {response: {body: "Not found"} })

        //  Handle a request with an inline function
        r.add("unknown", "/oldStuff/", {run: function(request) { return {body: "Not found"} }})

        //  A custom matching function to match SSL requests
        r.add("secure", function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }, {name: "secure", action: "private" })

        //  A matching function that rewrites a request and then continues matching other routes
        r.add("upgrade", function (request) {
            if (request.uri.startsWith("/old")) {
                request.uri = request.uri.toString().trimStart("/old")
                return false
            })

        //  Nest matching routes
        r.add("posts", "/blog", {controller: "post", subroute: {
                r.add("comment", "/comment/:action/:id")
            }
        })

        //  Match with regular expression. The sub-match is available via $N parameters
        r.add("dash", /^\/Dash-((Mini)|(Full))/, { controller: "post", action: "list", kind: "$1" })
        
        //  Conditional matching. Surround optional tokens in "()"
        r.add("dash", "/Dash(/:product(/:branch(/:configuration)))", {   
            name: "dash", 
            method: "GET", 
            controller: "Dash", 
            action: "index",
            after: "home",
        })

        @stability prototype
        @spec ejs
     */
    class Router {

        public static const Restful = "restful"
        public static const Direct = "direct"
        public static const Top = "top"

        /* Seed for generating route names */
        var nameSeed: Number = 0

        /*
            Master Route Table. Routes are processed from first to last. Inner routes are tested before their outer parent.
         */
		var routes: Array = []
		var routeLookup: Object = {}
		
        /*
            Map of run functions based on request extension 
         */
        var runners = {
            "es":   ScriptBuilder,
            "ejs":  TemplateBuilder,
            "html": StaticBuilder,
        }

        /**
            Function to test if the Request.filename is a directory.
            @param request Request object to consider
            @return True if request.filename is a directory
         */
        public static function isDir(request) request.filename.isDir

        /**
            Add top-level routes for static content, directories and "es" and "ejs" scripts.
         */
        public function topRoutes() {
            add("home", /^\/$/,   {run: StaticBuilder, redirect: "/index.ejs"})
            add("es",   /\.es$/,  {run: ScriptBuilder})
            add("ejs",  /\.ejs$/, {module: "ejs.template", run: TemplateBuilder})
            add("dir",  isDir,    {run: DirBuilder})
        }

        /**
            Add a default route for static content not matched by prior routes
         */
        public function defaultRoute() {
            add("default", null, {run: StaticBuilder})
        }
//ZZZ
        public function resourceRoutes(name: String, prefix: String = null): Void {
            name = name.trimStart(":")
            if (prefix === null) {
                prefix = (name + "-")
            }
            add(prefix + "new",     "/" + name + "/new",      {run: MvcBuilder,  method: "GET",  action: "new"})
            add(prefix + "edit",    "/" + name + "/:id/edit", {run: MvcBuilder,  method: "GET",  action: "edit"})
            add(prefix + "show",    "/" + name + "/:id",      {run: MvcBuilder,  method: "GET",  action: "show"})
            add(prefix + "update",  "/" + name + "/:id",      {run: MvcBuilder,  method: "PUT",  action: "update"})
            add(prefix + "delete",  "/" + name + "/:id",      {run: MvcBuilder,  method: "DELETE", action: "delete"})
            add(prefix + "default", "/" + name + "/:action",  {run: MvcBuilder})
            add(prefix + "create",  "/" + name + "",          {run: MvcBuilder,  method: "POST", action: "create"})
            add(prefix + "index",   "/" + name + "",          {run: MvcBuilder,  method: "GET",  action: "index"})
        }

        /** 
            Restful routes for MVC apps. Supports CRUD actions: index, show, create, update, destroy. 
            The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET		/controller             index
                POST	/controller/new         new         NEED new name
                POST	/controller             create      Create new 
                GET		/controller/1           show        Get and display data (not editable)
                GET		/controller/1/edit      edit        Get and display a form
                PUT		/controller/1           update      Update 
                DELETE	/controller/1           destroy     
            </pre>
            The default route is used for $Request.link.
        */
        public function restfulRoutes() {
            topRoutes()
            let staticPattern = RegExp("^\\/" + (App.config.directories.static || "static") + "\\/")
            add("static",   staticPattern, {run: StaticBuilder})
            resourceRoutes(":controller", "")
            defaultRoute()
        }

        /** 
            Direct routes for MVC apps. These map HTTP methods directly to namespace qualified controller methods.
            Supports CRUD actions: index, show, create, update, destroy. 
            The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET		/controller             index
                POST	/controller/new         new         NEED new name
                POST	/controller             create      Create new 
                GET		/controller/1           show        Get and display data (not editable)
                GET		/controller/1/edit      edit        Get and display a form
                PUT		/controller/1           update      Update 
                DELETE	/controller/1           destroy     
            </pre>
            The default route is used for $Request.link.
        */
        public function directRoutes() {
            topRoutes()
            let staticPattern = RegExp("^\/" + (App.config.directories.static || "static") + "\/")
            add("static",   staticPattern,          {run: StaticBuilder})
            add("new",      "/:controller/new",     {run: MvcBuilder,    method: "GET",  action: "GET::new"})
            add("edit",     "/:controller/:id/edit",{run: MvcBuilder,    method: "GET",  action: "GET::thing"})
            add("show",     "/:controller/:id",     {run: MvcBuilder,    method: "GET",  action: "GET::thing"})
            add("update",   "/:controller/:id",     {run: MvcBuilder,    method: "PUT",  action: "PUT::thing"})
            add("delete",   "/:controller/:id",     {run: MvcBuilder,    method: "DELETE", action: "DELETE::thing"})
            add("default",  "/:controller/:action", {run: MvcBuilder})
            add("create",   "/:controller",         {run: MvcBuilder,    method: "POST", action: "POST::create"})
            add("index",    "/:controller",         {run: MvcBuilder,    method: "GET",  action: "GET::index"})
            defaultRoute()
        }

        function Router(kind: String = Router.Restful) {
            switch (kind) {
            case "direct":
                directRoutes()
                break
            case "restful":
                restfulRoutes()
                break
            case "top":
                topRoutes()
                break
            default:
                throw "Unknown route kind"
            }
        }

        function addBuilder(builder: Function, ext: String): Void
            runners[ext] = builder

        function lookupRunners(ext): Function {
            let runner = runners[ext] || MvcBuilder
            return runner
        }

        /**
            Add a route
            @param pattern Set of routes to add. This must be an array of Route instances.
            @param options
            @return The route name
MOB - doc
            @option action String Short form for params.action
            @option builder Outer parent route
            @option do String Short form for params.controller "#" params.action
            @option name Outer parent route
            @option method Outer parent route
            @option limits Outer parent route
            @option location Object hash with properties prefix and dir
            @option params Outer parent route
            @option parent Outer parent route
            @option redirect 
            @option rewrite 
            @option run (Function|Object) This can be either a function to serve the request or it can be a 
                response hash with status, headers and body properties. The function should return such a response object.
         */
		public function add(name: String, pattern, options: Object = {}): String {
            let r = new Route(this)

            name ||= (nameSeed++ cast String)
            r.name = name

            /* 
                Combine with all outer routes. Outer route patterns are prepended. Order matters. 
             */
            let outer = options.parent
            while (outer) {
                if (r.name) {
                    r.name = outer.name + "." + r.name
                }
                pattern = outer.match + pattern
                for (p in outer.params) {
                    r.params[p] = outer.params[p]
                }
                outer = outer.parent
            }
//  MOB -- functionalize
            /*  
                Compile the route and create a RegExp matcher if the match pattern is a string. Each :token is 
                extracted into tokens and a corresponding RegExp sub-expression is created in the matcher.
             */
            let splitter, tokens, method

            r.match = pattern
            r.method = options.method

            if (pattern is String) {
                /*  
                    For string patterns, Create a regular expression splitter pattern so :TOKENS can be referenced
                    positionally in the override hash via $N args.
                 */
print("BEFORE " + pattern)
                pattern = pattern.replace(/\)/g, ")*")
print("AFTER " + pattern)
                tokens = pattern.match(/:([^:\W]*)/g)
                for (i in tokens) {
                    tokens[i] = tokens[i].trim(":")
                }
                let template = pattern.replace(/:([^:\W]+)/g, "([^\W]*)").replace(/\//g, "\\/")
                if (!template.contains(".:format")) {
                    template += "(.:format)*"
                }
print("TEMPLATE " + template)
                r.matcher = RegExp("^" + template)
                /*  Splitter ends up looking like "$1$2$3$4..." */
                count = 1
                splitter = ""
                for (c in tokens) {
                    splitter += "$" + count++ + ":"
                }
                r.splitter = splitter.trim(":")
                r.tokens = tokens
            } else {
                if (pattern is Function) {
                    r.matcher = pattern
                } else if (pattern is RegExp) {
                    r.matcher = pattern
                } else {
                    r.matcher = RegExp(pattern.toString())
                }
            }
            if (options.middleware) {
//  MOB -- needs testing
                r.middleware = options.middleware.reverse()
            }
            let params = r.params = options.params || {}
            if (options.action) {
                params.action = options.action
            }
            if (options.controller) {
                params.controller = options.controller
            }
//  MOB -- fix options.do
            if (options.dox) {
                let [controller, action] = options.dox.split("#")
                params.action = action
                params.controller = controller
            }
            if (params.action) {
                let [ns, action] = params.action.split("::")
                if (action) {
                    params.action = action
                    params.ns = ns
                } else {
                    params.action = ns
                }
            }
            r.run = options.run || lookupRunners(r)
            if (!(r.run is Function)) {
                r.run = function(request) {
                    return options.run
                }
            }
            r.options = options

            if (options.parent) {
                /* Must process nested routes first before appending the parent route to the routes table */
                //  MOB -- needs work
                options.parent.parent = r
                addRoutes(r.subroute, r)
            }
            routeLookup[r.name] = r

            let inserted
            if (options.replace) {
                for (let [i, route] in routes) {
                    if (route.name == options.replace) {
                        routes.remove(i, i)
                        inserted = routes.insert(i, r)
                        break
                    }
                }
            } else if (options.first) {
                inserted = routes.insert(0, r)
            } else if (options.before) {
                for (let [i, route] in routes) {
                    if (route.name == options.before) {
                        inserted = routes.insert(i, r)
                        break
                    }
                }
            } else if (options.after) {
                for (let [i, route] in routes) {
                    if (route.name == options.after) {
                        inserted = routes.insert(i + 1, r)
                        break
                    }
                }
            }
            if (!inserted) {
                routes.append(r)
            }
            return r.name
		}

		public function replace(name: String, pattern, options: Object = {}): Void {
            options.replace = name
            add(name, pattern, options)
        }

		public function remove(name: String): Void {
            for (i in routes) {
                if (routes[i].name == name) {
                    routes.remove(i)
                    break
                }
            }
        }


        /** 
            Route a request. The request is matched against the user-configured route table. If no route table is defined,
            the restfulRoutes are used. The call returns the web application to execute.
            @param request The current request object
            @return The web application function of the signature: 
                function app(request: Request): Object
         */
        public function route(request): Function {
            let params = request.params
            let pathInfo = request.pathInfo
            let log = request.log
            log.debug(5, "Routing " + request.pathInfo)

            if (request.method == "POST") {
                let method = request.params["__method__"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method && method.toUpperCase() != request.method) {
                    // MOB automatically done request.originalMethod ||= request.method
                    log.debug(3, "Change method from " + request.method + " TO " + method + " for " + request.uri)
                    request.method = method
                }
            }

print("PI \"" + pathInfo + "\"")

            //  MOB - need better way to turn on debug trace without slowing down the router
            for each (r in routes) {
                let options = r.options
                log.debug(6, "Test route \"" + r.name + "\"")
print("Test route " + r.name)

                if (r.method) {
                    if (!request.method.contains(r.method)) {
                        continue
                    }
                }
print("MATCHER " + r.matcher)
                if (r.matcher is Function) { 
                    if (!r.matcher(request)) {
                        continue
                    }
                    for (i in r.params) {
                        params[i] = r.params[i]
                    }

                } else if (!r.splitter) { 
                    if (r.matcher) {
                        let results = pathInfo.match(r.matcher)
                        if (!results) {
                            continue
                        }
                        for (let name in r.params) {
                            let value = r.params[name]
                            if (value.contains("$")) {
                                value = pathInfo.replace(r.matcher, value)
                            }
                            params[name] = value
                        }
                    } else {
                        for (i in r.params) {
                            params[i] = r.params[i]
                        }
                    }

                } else {
print("STRING/REGEXP " + pathInfo)
                    /*  String or RegExp based matcher */
                    if (!pathInfo.match(r.matcher)) {
print("STRING/REGEXP - continue")
                        continue
                    }
                    parts = pathInfo.replace(r.matcher, r.splitter)
print("SPLITTER " + r.splitter)
print("TOKENS " + r.tokens)
print("PARTS " + parts)
                    parts = parts.split(":")
                    for (i in r.tokens) {
                        params[r.tokens[i]] = parts[i]
                    }
                    /*  Apply override params */
                    for (i in r.params) {
                        params[i] = r.params[i]
                    }
                }
                if (r.rewrite && !r.rewrite(request)) {
                    log.debug(5, "Request rewritten as \"" + request.pathInfo + "\" (reroute)")
                    return route(request)
                }
                if (r.redirect) {
                    request.pathInfo = r.redirect
                    log.debug(5, "Route redirected to \"" + request.pathInfo + "\" (reroute)")
                    return route(request)
                }
                request.route = r
                let location = r.location
                if (location && location.prefix && location.dir) {
                    request.setLocation(location.prefix, location.dir)
                    log.debug(4, "Set location prefix \"" + location.prefix + "\" dir \"" + location.dir + "\" (reroute)")
                    return route(request)
                }
                if (options.module && !r.initialized) {
                    global.load(options.module + ".mod")
                    r.initialized = true
                }
print("@@@@ Matched route \"" + r.name + "\"")
dump("PARAMS", request.params)
                if (log.level >= 4) {
                    log.debug(4, "Matched route \"" + r.name + "\"")
                    log.debug(5, "  Route params " + serialize(params, {pretty: true}))
                    if (log.level >= 6) {
                        log.debug(6, "  Route " + serialize(r, {pretty: true}))
                        log.debug(6, "  REQUEST\n" + serialize(request, {pretty: true}))
                    }
                }
                if (r.limits) {
                    request.setLimits(r.limits)
                }
                if (r.trace) {
                    if (r.trace.include && (!r.trace.include.contains(request.extension)) ||
                        r.trace.exclude && r.trace.exclude.contains(request.extension)) {
                        request.trace(99)
                    } else {
                        request.trace(r.trace.level || 0, r.trace.options, r.trace.size)
                    }
                }
                r.params.ns ||= request.method
                let app = r.run(request)

                if (app == null) {
                    return function(request) {}
                }
                return app
            }
            throw "No route for " + pathInfo
        }

        public function show(all: Boolean = false): Void {
            // dump(routes)
            print("Route Table:")
            for each (r in routes) {
                let o = r.options
                let method = o.method || "ALL"
                let target
                if (o.controller || o.action) {
                    let controller = o.controller || "*"
                    target = controller + "." + o.action
                } else if (r.run) {
                    target = r.run.name
                } else {
                    target = "UNKNOWN"
                }
                let match = r.match
                if (match is Function) {
                    match = match.name
                } else if (!match) {
                    match = "*"
                }
                let line = "%10s: %16s: %7s: %-45s".format(r.name, target, method, match)
                if (all) {
                    if (r.params && Object.getOwnPropertyCount(r.params) > 0) {
                        if (!(r.params.action && Object.getOwnPropertyCount(r.params) == 1)) {
                            line += "  %s".format(serialize(r.params))
                        }
                    }
                }
                print(line)
                /*
                    module
                    middleware
                    params
                    subroute
                    splitter
                 */
            }
            print()
        }
    }

//  MOB - move this back to Web?
    /** 
        Script builder for use in routing tables to load pure script files (*.es).
        @param request Request object. 
        @return A web script function that services a web request.
        @example:
          { name: "index", builder: ScriptBuilder, match: "\.es$" }
     */
    function ScriptBuilder(request: Request): Function {
        if (!request.filename.exists) {
            request.writeError(Http.NotFound, "Cannot find " + request.pathInfo) 
            return null
        }
        try {
            return Loader.require(request.filename, request.config).app
        } catch (e) {
            request.writeError(Http.ServerError, e)
        }
    }

    /** 
        Route class. A Route describes a mapping from a set of resources to a URI. The Router uses tables of 
        Routes to serve and route requests to web scripts.

        If the URL pattern is a regular expression, it is used to match against the request pathInfo. If it matches,
        the pathInfo is matched and sub-expressions may be referenced in the override parameters by using $1, $2 and
        so on. e.g. { priority: "$1" }
        
        If the URL pattern is a function, it is run to test for a request match. It should return true to 
        accept the request. The function can set parameters in request.params.

        The optional override hash provides parameters which will be defined in params[] overriding any tokenized 
        parameters previously set.
      
        Routes are tested in-order from first to last. Inner routes are tested before their outer parent.
     */
    enumerable dynamic class Route {
        use default namespace public

        /**
            Matching pattern for URIs. The pattern is used to match the request in general and pathInfo specifically. 
            The pattern can be a token string, a regular expression or a function. If it is a string of tokens separated
            by ":", it is converted to a regular expression and the positional tokens (:NAME) are extracted for web
            requests and mapped to items in the params collection. ie. params[NAME]. 

            If the match pattern is a regular expression, it is used to match against the request pathInfo. If it matches
            the pathInfo, then sub-expressions may be referenced in the $params values by using $1, $2 and
            so on. e.g. params: { priority: "$1" }
            
            If the match pattern is a function, it is run to test for a request match. The function should return true to 
            match the request. The function can set parameters in request.params.
        */
        var match: Object

        /**
            HTTP method to match. If null, all methods are matched
         */
        var method: String

        /**
            Middleware to run on requests for this route. Middleware wraps the application function filtering and 
            modifying its inputs and outputs.
         */
        var middleware: Array

        /**
            Route name
         */
        var name: String

        /**
            Extra route options
            @option limits Object Resource limits for the request. See HttpServer.limits for details.
            @option linker Function Function that will be called by Request.link to make to make URI links.
            
                function linker(uri: Uri, request: Request, options: Object): Uri
            @option location Object Application location to serve the request. Location contains two properties: prefix
                which is the string URI prefix for the application and dir which is a Path to the physical file system
                directory containing the MVC applciation.
            @option String Name of the required module containing code to serve requests on thsi route.  
            @option rewrite Function Rewrite function to rewrite the request before serving. It may update
                the request scriptName, pathInfo and other Request properties.
            @option redirect String URI to redirect the request toward.
            @option threaded Boolean If true, the request should be run in a worker thread if possible. This thread
                will not be dedicated, but will be assigned as the request requires CPU resources.
            @option trace Object Trace options for the request. Note: the route is created after the Request object 
                is created so the tracing of the connections and request headers will be controlled by the owning server. 
                See HttpServer.trace for trace property fields.
         */
        var options: Object

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /** 
          Router instance reference
         */
        var router: Router

        /**
            Function to run to serve the request.
         */
        var runner: Function

        /**
            Nested route. A nested route prepends the match patter of the outer route to its "match" pattern. 
            The param set of the outer route are appended to the inner route.
         */
        var subroute: Route

/////////////////////// UNUSED

//        /*
//            Type of requests matched by this route. Typical types: "es", "ejs", "mvc"
//         */
//        var type: String
//
//        /**
//            Directory for the application serving the route. This directory path will be assigned to Request.dir.
//         */
//        var dir: Path
//        /**
//            Provider function that represents the web application for requests matching this route
//         */
//        var provider: Function

        internal var matcher: Object
        internal var splitter: String
        internal var tokens: Array

        function Route(router: Router) {
            this.router = router
        }

        /**
            Complete a URI link by adding route token information. 
            @param uri Current URI to complete
            @param request Current request object
            @return A Uri object.
         */
        public function completeLink(uri: Uri, request: Request, target: Object): Uri {
            if (options.linker) {
                return options.linker(uri, request, target)
            }
            let routeName = options.route || "default"
            let route = this
            if (routeName != this.name) {
                /* Using another route */
                route = router.routeLookup[routeName]
                if (!route) {
                    throw new ReferenceError("link: Unknown route \"" + routeName + "\"")
                }
            }
            if (route.tokens) {
                for each (let token in route.tokens) {
                    let value = target[token]
                    if (value == undefined) {
                        if (token == "action") {
                            continue
                        }
                        value = request.params[token]
                        if (value == undefined) {
                            throw new ArgError("Missing URI token \"" + token + "\"")
                        }
                    }
                    uri = uri.join(value)
                }
            }
            return uri
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
