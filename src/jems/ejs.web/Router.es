/**
    Router.es - Web Request router. Parse and route web requests.
 */

module ejs.web {

    /** 
        Web router class. Routes incomding HTTP requests to the appropriate location. The Route class supports 
        configurable user-defined route tables. It parses the route table at startup and matches request URIs and other 
        request parameters to determine the matching application and target for the request.
        @stability prototype
        @spec ejs
     */
    class Router {
        /*
            Master Route Table. Routes are processed from first to last. Inner routes are tested before their outer parent.
         */
		var routes: Array = []
		
        public static function isDir(request) request.filename.isDir

        /**
            Simple top level route table for "es" and "ejs" scripts. Matches simply by script extension.
         */
        public static var TopRoutes = [
          { name: "es",      builder: ScriptBuilder,    match: /\.es$/ },
          { name: "ejs",     builder: TemplateBuilder,  match: /\.ejs$/, module: "ejs.template" },
          { name: "dir",     builder: DirBuilder,       match: isDir },
          { name: "default", builder: StaticBuilder },
        ]

        /** 
            Restful routes. Supports CRUD actions: index, show, create, update, destroy. The restful routes defined are:
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
        */
        public static var RestfulRoutes = [
  { name: "es",      builder: ScriptBuilder,                match: /^\/web\/.*\.es$/   },
  { name: "ejs",     builder: TemplateBuilder,              match: /^\/web\/.*\.ejs$/,      module: "ejs.template" },
  { name: "web",     builder: StaticBuilder,                match: /^\/web\//  },
  { name: "home",    builder: StaticBuilder,                match: /^\/$/,                  redirect: "/web/index.ejs" },
  { name: "ico",     builder: StaticBuilder,                match: /^\/favicon.ico$/,       redirect: "/web/favicon.ico" },
  { name: "dir",     builder: DirBuilder,                   match: isDir },
  { name: "new",     builder: MvcBuilder, method: "GET",    match: "/:controller/new",      params: { action: "new" } },
  { name: "edit",    builder: MvcBuilder, method: "GET",    match: "/:controller/:id/edit", params: { action: "edit" } },
  { name: "show",    builder: MvcBuilder, method: "GET",    match: "/:controller/:id",      params: { action: "show" } },
  { name: "update",  builder: MvcBuilder, method: "PUT",    match: "/:controller/:id",      params: { action: "update" } },
  { name: "delete",  builder: MvcBuilder, method: "DELETE", match: "/:controller/:id",      params: { action: "delete" } },
  { name: "default", builder: MvcBuilder,                   match: "/:controller/:action",  params: {} },
  { name: "create",  builder: MvcBuilder, method: "POST",   match: "/:controller",          params: { action: "create" } },
  { name: "index",   builder: MvcBuilder, method: "GET",    match: "/:controller",          params: { action: "index" } },
        ]

        public static var LegacyRoutes = [
  { name: "es",      builder: ScriptBuilder,                match: /^\/web\/.*\.es$/   },
  { name: "ejs",     builder: TemplateBuilder,              match: /^\/web\/.*\.ejs$/,      module: "ejs.template"  },
  { name: "web",     builder: StaticBuilder,                match: /^\/web\//  },
  { name: "home",    builder: StaticBuilder,                match: /^\/$/,                  redirect: "/web/index.ejs" },
  { name: "ico",     builder: StaticBuilder,                match: /^\/favicon.ico$/,       redirect: "/web/favicon.ico" },
  { name: "list",    builder: MvcBuilder, method: "GET",    match: "/:controller/list",     params: { action: "list" } },
  { name: "create",  builder: MvcBuilder, method: "POST",   match: "/:controller/create",   params: { action: "create" } },
  { name: "edit",    builder: MvcBuilder, method: "GET",    match: "/:controller/edit",     params: { action: "edit" } },
  { name: "update",  builder: MvcBuilder, method: "POST",   match: "/:controller/update",   params: { action: "update" } },
  { name: "destroy", builder: MvcBuilder, method: "POST",   match: "/:controller/destroy",  params: { action: "destroy" } },
  { name: "default", builder: MvcBuilder,                   match: "/:controller/:action",  params: {} },
  { name: "index",   builder: MvcBuilder, method: "GET",    match: "/:controller",          params: { action: "index" } },
  { name: "static",  builder: StaticBuilder, },
        ]

        function Router(set: Array = RestfulRoutes) {
            addRoutes(set)
        }

        /**
            Add the restful routes to the routing table
         */
		public function addRestfulRoutes(): Void
            addRoutes(RestfulRoutes)

        /**
            Add a set of routes to the routing table
            @param routeSet Set of routes to add. This must be an array of Route instances.
            @param outer Outer route
         */
		public function addRoutes(routeSet: Array, outer: Route? = null): Void {
            for each (route in routeSet) {
                route = route.clone()
                /* 
                    Combine with all outer routes. Outer route patterns are prepended. Order matters. 
                 */
                while (outer) {
                    route.name = outer.name + "." + route.name
                    route.match = outer.match + route.match
                    for (p in outer.params) {
                        route.params[p] = outer.params[p]
                    }
                    outer = outer.parent
                }
                /*  
                    Compile the route and create a RegExp matcher if the match pattern is a string. Each :token is 
                    extracted into tokens and a corresponding RegExp sub-expression is created in the matcher.
                 */
                let splitter, tokens
                route.matcher = route.match
                if (route.match is String) {
                    /*  
                        For string patterns, Create a regular expression splitter pattern so :TOKENS can be referenced
                        positionally in the override hash via $N args.
                     */
                    tokens = route.match.match(/:([^:\W]*)/g)
                    for (i in tokens) {
                        tokens[i] = tokens[i].trim(":")
                    }
                    let template = route.match.replace(/:([^:\W]+)/g, "([^\W]*)").replace(/\//g, "\\/")
                    route.matcher = RegExp("^" + template)
                    /*  Splitter ends up looking like "$1$2$3$4..." */
                    count = 1
                    splitter = ""
                    for (c in tokens) {
                        splitter += "$" + count++ + ":"
                    }
                    route.splitter = splitter.trim(":")
                    route.tokens = tokens
                }
                if (route.middleware) {
                    route.middleware = route.middleware.reverse()
                }
                route = new Route(route, this)
                if (route.subroute) {
                    /* Must process nested routes first before appending the parent route to the routes table */
                    route.subroute.parent = route
                    addRoutes(route.subroute, route)
                }
                routes.append(route)
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

            //  MOB - need better way to turn on debug trace without slowing down the router
            for each (r in routes) {
                log.debug(6, "Test route \"" + r.name + "\"")

                if (r.method && request.method != r.method) {
                    continue
                }
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
                    /*  String or RegExp based matcher */
                    if (!pathInfo.match(r.matcher)) {
                        continue
                    }
                    parts = pathInfo.replace(r.matcher, r.splitter)
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
                    request.pathInfo = r.redirect;
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
                if (r.module && !r.initialized) {
                    global.load(r.module + ".mod")
                    r.initialized = true
                }
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
                let app = r.builder(request)
                if (app == null) {
                    return function(request) {}
                }
                return app
            }
            throw "No route for " + pathInfo
        }
    }

    /**
        Builder function to load JSGI scripts.
        @return JSGI application function 
      */
    function ScriptBuilder(request: Request): Function {
        if (!request.filename.exists) {
            request.error(Http.NotFound, "Cannot find " + request.pathInfo) 
            return null
        }
        try {
            return Loader.require(request.filename, request.config).app
        } catch (e) {
            request.error(Http.ServerError, e)
        }
    }

    function TemplateBuilder(request: Request): Function {
        let route = request.route
        if (route.module && !route.initialized) {
            global.load(route.module + ".mod")
            route.initialized = true
        }
        return "ejs.web"::TemplateApp
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
            Builder function to create the Provider function that represents the web application for requests matching 
            this route.
         */
        var builder: Function

        /**
            Directory for the application serving the route. This directory path will be assigned to Request.dir.
         */
        var dir: Path

        /**
            Resource limits for the request. See HttpServer.limits for details.
          */
        var limits: Object

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
            Module containing code to serve the route.  
         */
        var module: String

        /**
            Optional route name.
         */
        var name: String

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Provider function that represents the web application for requests matching this route
         */
        var provider: Function

        /**
            Rewrite function. If present, this function is invoked with the Request as an argument. It may rewrite
            the request scriptName, pathInfo and other Request properties.
         */
        var rewrite: Function

        /** 
          Router instance reference
         */
        var router: Router

        /**
            Nested route. A nested route prepends the match patter of the outer route to its "match" pattern. 
            The param set of the outer route are appended to the inner route.
         */
        var subroute: Route

        /**
            Threaded request indicator. If true, the request should be run in a worker thread if possible. This thread
            will not be dedicated, but will be assigned as the request requires CPU resources.
         */
        var threaded: Boolean

        /**
            Trace options for the request. Note: the route is created after the Request object is created so 
            the tracing of the connections and request headers will be controlled by the owning server. See
            HttpServer.trace. Fields are:
            @option level Level at which request tracing will occurr for the request.
            @option options Set of trace options. Select from: "body" to trace body content.
            @option size Maximum request body size to trace
            @option include Set of extensions to include when tracing
            @option exclude Set of extensions to exclude when tracing
          */
        var trace: Object

        /*
            Type of requests matched by this route. Typical types: "es", "ejs", "mvc"
         */
        var type: String

        /**
            Function to make URIs according to the route format
         */
        var urimaker: Function

        internal var matcher: Object
        internal var splitter: String
        internal var tokens: Array

        function Route(route: Object, router: Router) {
            for (field in route) {
               this[field] = route[field]
            } 
            this.router = router
        }

        //  MOB - OPT
        /**
            Make a URI provided parts of a URI. The URI is completed using the current request and route. 
            @param request Request object
            @param components MOB
         */
        public function makeUri(request: Request, components: Object): Uri {
            if (urimaker) {
                return urimaker(request, components)
            }
            let where
            if (request) {
                //  Base the URI on the current application home uri
                let base = blend(request.absHome.components, request.params)
                delete base.id
                delete base.query
                if (components is String) {
                    where = blend(base, { path: components })
                } else {
                    where = blend(base, components)
                }
            } else {
                where = components.clone()
            }
            if (where.id != undefined) {
                if (where.query) {
                    where.query += "&" + "id=" + where.id
                } else {
                    where.query = "id=" + where.id
                }
            }
            let uri = Uri(where)
            let routeName = where.route || "default"
            let route = this
            if (routeName != this.name) {
                for each (r in router.routes) {
                    if (r.name == routeName) {
                        route = r
                        break
                    }
                }
            }
            if (!components.path) {
                for each (token in route.tokens) {
                    if (!where[token]) {
                        throw new ArgError("Missing URI token \"" + token + "\"")
                    }
                    uri = uri.join(where[token])
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
