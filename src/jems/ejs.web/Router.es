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
		internal var routes: Array = []
		
        //  MOB -- finish
        //  MOB - need ajax support to do non-get / non-post methods in a client
        //  MOB -- should have default MVC route in here
        //  MOB -- should this be the default route and not RestfulRoutes?
        //  MOB -- can a route do a Uri rewrite
        //  MOB -- how normal static content be served?

        /**
            Simple top level route table for "es" and "ejs" scripts. Matches simply by script extension.
         */
        public static var TopRoutes = [
          { name: "es",      type: "es",     match: /\.es$/   },
          { name: "ejs",     type: "ejs",    match: /\.ejs$/  },
          { name: "default", type: "static", },
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
          { name: "new",     type: "mvc", method: "GET",    match: "/:controller/new",       params: { action: "new" } },
          { name: "edit",    type: "mvc", method: "GET",    match: "/:controller/:id/edit",  params: { action: "edit" } },
          { name: "show",    type: "mvc", method: "GET",    match: "/:controller/:id",       params: { action: "show" } },
          { name: "update",  type: "mvc", method: "PUT",    match: "/:controller/:id",       params: { action: "update" } },
          { name: "delete",  type: "mvc", method: "DELETE", match: "/:controller/:id",       params: { action: "delete" } },
          { name: "default", type: "mvc", method: "GET",  match: "/:controller/:action",     params: {} },
          { name: "create",  type: "mvc", method: "POST",   match: "/:controller",           params: { action: "create" } },
          { name: "index",   type: "mvc", method: "GET",    match: "/:controller",           params: { action: "index" } },
          { name: "home",    type: "static", match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "static",  type: "static", },
        ]

        /**
            Simple routes  - Not used
        public static var SimpleRoutes = [
          { name: "list",    type: "mvc", method: "GET",  match: "/:controller/list",        params: { action: "list" } },
          { name: "create",  type: "mvc", method: "POST", match: "/:controller/create",      params: { action: "create" } },
          { name: "edit",    type: "mvc", method: "GET",  match: "/:controller/:id",         params: { action: "edit" } },
          { name: "update",  type: "mvc", method: "POST", match: "/:controller/:id",         params: { action: "update" } },
          { name: "destroy", type: "mvc", method: "POST", match: "/:controller/destroy/:id", params: { action: "destroy" }},
          { name: "default", type: "mvc", method: "GET",  match: "/:controller/:action",     params: {} },
          { name: "index",   type: "mvc", method: "GET",  match: "/:controller",             params: { action: "index" } },
          { name: "home",    type: "static", match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "static",  type: "static", },
        ]
         */

        /**
            Routes used in Ejscript 1.X
         */
        //  MOB -- rename LegacyMvcRoutes
        public static var LegacyRoutes = [
          { name: "es",      type: "es",  match: /web\/.*\.es$/   },
          { name: "ejs",     type: "ejs", match: /web\/.*\.ejs$/  },
          { name: "web",     type: "static", match: /web\//  },
          { name: "list",    type: "mvc", method: "GET",  match: "/:controller/list",        params: { action: "list" } },
          { name: "create",  type: "mvc", method: "POST", match: "/:controller/create",      params: { action: "create" } },
          { name: "edit",    type: "mvc", method: "GET",  match: "/:controller/edit",        params: { action: "edit" } },
          { name: "update",  type: "mvc", method: "POST", match: "/:controller/update",      params: { action: "update" } },
          { name: "destroy", type: "mvc", method: "POST", match: "/:controller/destroy",     params: { action: "destroy" } },
          { name: "default", type: "mvc", method: "GET",  match: "/:controller/:action",     params: {} },
          { name: "index",   type: "mvc", method: "GET",  match: "/:controller",             params: { action: "index" } },
          { name: "home",    type: "static", match: /^\/$/, redirect: "/Base/home" },
          { name: "static",  type: "static", },
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
                    let template = route.match.replace(/:([^:\W]+)/g, "([^\W]+)").replace(/\//g, "\\/")
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
            the restfulRoutes are used. 
            @param request The current request object
         */
        public function route(request): Void {
            let params = request.params
            let pathInfo = request.pathInfo
            for each (r in routes) {
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
                    route(request)
                    return
                }
                if (r.redirect) {
                    request.pathInfo = r.redirect;
                    this.route(request)
                    return
                }
                request.route = r
                let location = r.location
                if (location && location.prefix && location.dir) {
                    request.setLocation(location.prefix, location.dir)
                }
                let log = request.log
                if (log.level >= 5) {
                    show(request.log)
                }
                return
            }
            throw "No route for " + pathInfo
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
    dynamic class Route {
        use default namespace public

        /**
            Directory for the application serving the route. This directory path will be assigned to Request.dir.
         */
        var dir: Path

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
            Optional route name.
         */
        var name: String

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Rewrite function. If present, this function is invoked with the Request as an argument. It may rewrite
            the request scriptName, pathInfo and other Request properties.
         */
        var rewrite: Function

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

        /*
            Type of requests matched by this route. Typical types: "es", "ejs", "mvc"
         */
        var type: String

        /**
            Function to make URIs according to the route format
         */
        var urimaker: Function

        internal var matcher: RegExp
        internal var splitter: String
        internal var tokens: Array
        internal var router: Router

        function Route(route: Object, router: Router) {
            for (field in route) {
               this[field] = route[field]
            } 
            this.router = router
        }

        public function show(log: Logger, msg) {
            log.debug(5, msg + " \"" + name + "\":")
            for each (f in Object.getOwnPropertyNames(this)) {
                if (f != "params" && f != "router") {
                    log.debug(5, "    " + f + " = " + this[f])
                }
            }
            log.debug(5, "    params = " + serialize(params))
        }

        /**
            Make a URI provided parts of a URI. The URI is completed using the current request and route. 
            @param where MOB
         */
        public function makeUri(request: Request, where: Object): Uri {
            if (urimaker) {
                return urimaker(request, where)
            }
            if (request) {
                if (where is String) {
                    where = blend(request.absHome.components(), { path: where })
                } else {
                    where = blend(request.absHome.components(), where)
                }
            }
            let result = Uri(where)
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
            let path = ""
            for each (token in route.tokens) {
                if (!where[token]) {
                    throw "Missing URI token " + token
                }
                path += "/" + where[token]
            }
            result.path = path
            return result
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
