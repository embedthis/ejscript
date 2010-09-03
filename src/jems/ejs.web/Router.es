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

        //  Add route for files with a ".es" extension and use the ScriptBuilder to run
        r.add("es", /\.es$/,  {run: ScriptBuilder})

        //  Add route for directories and use the DirBuilder to run
        r.add(Router.isDir,    {name: "dir", run: DirBuilder})

        //  Add route for RESTful routes and run with the MvcBuilder
        r.add("edit",    "/{controller}/{id}/edit", {method: "GET",  action: "edit"})
        r.add("show",    "/{controller}/{id}",      {method: "GET",  action: "show"})
        r.add("update",  "/{controller}/{id}",      {method: "PUT",  action: "update"})
        r.add("destroy", "/{controller}/{id}",      {method: "DELETE", action: "destroy"})
        r.add("init",    "/{controller}/init",      {method: "GET",  action: "init"})
        r.add("index",   "/{controller}",           {method: "GET",  action: "index"})
        r.add("create",  "/{controller}",           {method: "POST", action: "create"})
        r.add("default", "/{controller}(/{action}(/{id}))")
        
        //  Add route for upper or lower case "D" or "d". Run the default app: MvcBuilder
        r.add("refresh", "/[Dd]ash/refresh", {method: "GET", controller: "Dash", action: "refresh", after: "static"})

        //  Add route for an "admin" application. This sets the scriptName to "admin" expects an MVC application to be
        //  located at the directory "myApp"
        r.add("adminApp", "/admin/", {location: { prefix: "/control", dir: "my"})

        //  Rewrite a request for "old.html" to new.html
        r.add("old", "/web/old.html",  {rewrite: function(request) { request.pathInfo = "/web/new.html"}})

        //  Handle a request with a literal response
        r.add("unknown", "/oldStuff/", {run: {body: "Not found"} })

        //  Handle a request with an inline function
        r.add("unknown", "/oldStuff/", {run: function(request) { return {body: "Not found"} }})

        //  A custom matching function to match SSL requests
        r.add("secure", function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }, {
                name: "secure", action: "private" 
            }
        })

        //  A matching function that rewrites a request and then continues matching other routes
        r.add("upgrade", function (request) {
            if (request.uri.startsWith("/old")) {
                request.uri = request.uri.toString().trimStart("/old")
                return false
            }
        })

        //  Nest matching routes
        r.add("posts", "/blog", {controller: "post", subroute: {
                r.add("comment", "/comment/{action}/{id}")
            }
        })

        //  Match with regular expression. The sub-match is available via $N parameters
        r.add("dash", /^\/Dash-((Mini)|(Full))$/, {controller: "post", action: "list", kind: "$1"})
        
        //  Conditional matching. Surround optional tokens in "()"
        r.add("dash", "/Dash(/{product}(/{branch}(/{configuration})))", {   
            name: "dash", 
            method: "GET", 
            controller: "Dash", 
            action: "index",
            after: "home",
        })

        //  Replace the home page route
        r.addHome("Status.index")

        //  Display the route table to the console
        r.show()

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
        var resources: Object = {}
        
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
            Add a catch-all route for static content
            @return The router instance to enable chaining
         */
        public function addCatchall(): Void
            add("catchall", /^\/.*$/, {run: StaticBuilder})

        /** 
            Direct routes for MVC apps. These map HTTP methods directly to method names.
        */
        public function addDirect(name: String, options: Object = {}): Void {
            add(name + "-solo", "/" + name + "/(/{id}(/{action}))")
            let names = options.plural || toPlural(name)
            //  MOB -- what should the group really be?
            add(name + "-collection", "/" + names + "/(/{action})", {contrtoller: name, namespace: "GROUP"})
        }

        /**
            Add a home page route. This will add or update the home page route.
            @return The router instance to enable chaining
         */
        public function addHome(options: String): Void
            add("home", "/", options)

        /**
            Add restful routes for a singleton resource. 
            Supports member CRUD actions: show, create, update, destroy.
            Supports collection actions: edit, index, show. 
            The restful routes defined are:
            <pre>
                Method  URL                   Action
                GET     /controller           show        Display a resource (not editable)
                GET     /controller/edit      edit        Display a resource form suitable for editing
                PUT     /controller           update      Update a resource (idempotent)
                DELETE  /controller           destroy     Destroy a resource (idempotent)
                ANY     /controller/action    *           Other custom actions

                GET     /controllers          index       Display an overview of the resource
                GET     /controllers/init     init        Initialize and display a blank form for a new resource
                POST    /controllers          create      Accept a form creating a new resource
                ANY     /controllers/action   *           Other custom actions
            </pre>
            The default route is used for $Request.link.
            @param name Singular name of the resource to route
            @param options Object hash of options. Defaults to {}
            @option controller Controller name to use instead of $name
            @return The router instance to enable chaining
         */
        public function addResource(name: String, options: Object = {}): Void {
            let c = options.controller || name
            add("edit",    '/'+name+ "/edit",      {resource: name, method: "GET",    controller: c, action: "edit"})
            add("show",    '/'+name,               {resource: name, method: "GET",    controller: c, action: "show"})
            add("update",  '/'+name,               {resource: name, method: "PUT",    controller: c, action: "update"})
            add("destroy", '/'+name,               {resource: name, method: "DELETE", controller: c, action: "destroy"})
            add("default", '/'+name+ "/{action}",  {resource: name,                   controller: c})
            
            let names = options.plural || toPlural(name)
            add("init",    '/'+names+ "/init",     {resource: name, method: "GET",    controller: c, action: "init"})
            add("index",   '/'+names,              {resource: name, method: "GET",    controller: c, action: "index"})
            add("create",  '/'+names,              {resource: name, method: "POST",   controller: c, action: "create"})
            add("default", '/'+names+ "/{action}", {resource: name,                   controller: c})
        }

        /** 
            Add restful routes for a resource collection. 
            Supports CRUD actions: edit, index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET     /controller/1           show        Display a resource (not editable)
                GET     /controller/1/edit      edit        Display a resource form suitable for editing
                PUT     /controller/1           update      Update a resource (idempotent)
                DELETE  /controller/1           destroy     Destroy a resource (idempotent)

                GET     /controller             index       Display an overview of the resource
                GET     /controller/init        init        Initialize and display a blank form for a new resource
                POST    /controller             create      Accept a form creating a new resource
                ANY     /controller             destroy     Destroy all (idempotent)
            </pre>
            The default route is used for $Request.link.
            @param resource Plural name of the resource to route
            @param options Object hash of options. Defaults to {}
            @option controller Controller name to use instead of $name
        */
        public function addResources(name: String, options: Object = {}): Void {
            let c = options.controller || name
            add("edit",    '/'+name + "/{id}/edit",  {resource: name, method: "GET",    controller: c, action: "edit"})
            add("show",    '/'+name + "/{id}",       {resource: name, method: "GET",    controller: c, action: "show"})
            add("update",  '/'+name + "/{id}",       {resource: name, method: "PUT",    controller: c, action: "update"})
            add("destroy", '/'+name + "/{id}",       {resource: name, method: "DELETE", controller: c, action: "destroy"})
            add("intit",   '/'+name + "/init",       {resource: name, method: "GET",    controller: c, action: "init"})
            add("create",  '/'+name,                 {resource: name, method: "POST",   controller: c, action: "create"})
            add("default", '/'+name + "/{action}",   {resource: name,                   controller: c})
        }

        //  Helpers get(), post(), destroy(), put()

        /** 
            Add default restful routes for singleton resources. This also adds top level routes and a static content
            catchall route.  @see $addResource for restful route details.
        */
        public function addRestful(): Void {
            //  Default resource routes for a singleton
            add("edit",    "/{controller}/edit",    {method: "GET",    action: "edit"})
            add("show",    "/{controller}",         {method: "GET",    action: "show"})
            add("update",  "/{controller}",         {method: "PUT",    action: "update"})
            add("destroy", "/{controller}",         {method: "DELETE", action: "destroy"})
            //  Default resource Collection routes 
            add("init",    "/{controller}/init",    {method: "GET",    action: "init"})
            add("create",  "/{controller}",         {method: "POST",   action: "create"})
            add("default", "/{controller}(/{action}(/{id}))")
        }

        /**
            Add simple MVC routes.
            specified controller. All HTTP method verbs are supported.
            @return The router instance to enable chaining
         */
        public function addSimple(): Void
            add("default", "/{controller}(/{action}(/{id}))")

        public function addStatic(): Void {
            let staticPattern = RegExp("^\\/" + (App.config.directories.static || "static") + "\\/")
            add("static", staticPattern, {run: StaticBuilder})
        }

        /**
            Add top-level routes for static content, directories and "es" and "ejs" scripts.
            @return The router instance to enable chaining
         */
        public function addTop(): Void {
            add("home", /^\/$/,   {run: StaticBuilder, redirect: "/index.ejs"})
            add("es",   /\.es$/,  {run: ScriptBuilder})
            add("ejs",  /\.ejs$/, {module: "ejs.template", run: TemplateBuilder})
            add("dir",  isDir,    {run: DirBuilder})
            addCatchall()
        }

        function Router(name: String = null) {
            switch (name) {
            case "direct":
                addDirect()
                break
            case "restful":
                addRestful()
                break
            case "top":
                addTop()
                break
            case null:
            case "":
                break
            default:
                throw "Unknown route set: " + name
            }
        }

        function addBuilder(builder: Function, ext: String): Void
            runners[ext] = builder

        function lookupRunners(ext): Function {
            let runner = runners[ext] || MvcBuilder
            return runner
        }

        /**
            Add a route.
            A route template must match the entire request pathInfo. 
            @param template Route template to match. If template is not supplied, the name is used as the template and is 
                interpreted as "controller(/action)".
            @param options
            @return The route name
MOB - doc
            @option action String Short form for params.action
            @option builder Outer parent route
            @option name Outer parent route
            @option method Outer parent route
            @option limits Outer parent route
            @option location Object hash with properties prefix and dir
            @option params Outer parent route
            @option parent Outer parent route
            @option resource String Name of the RESTful resource owning this route. The actual route name will use this
                resource name as a prefix.
            @option redirect 
            @option rewrite 
            @option run (Function|Object) This can be either a function to serve the request or it can be a 
                response hash with status, headers and body properties. The function should return such a response object.
         */
        public function add(name: String, template = null, options: Object = {}): Void {
            let r = new Route(this)

            if (options && options.resource) {
                name = options.resource + "-" + name
            }
            if (template == null) {
                //  Interpret as "/controller(/action)" and convert into "/:controller(/:action)
                template = "{" + name.split("/").join("}{") + "}"
            }
/*
            if (options === null) {
                options = { action: name }
            } else if (options is String) {
                options = { action: options }
            } else if (options.action == null) {
                options.action = name
            }
*/
            if (options is String) {
                options = { action: options }
            }
            name ||= (nameSeed++ cast String)
            r.name = name
            /* 
                Combine with all outer routes. Outer route templates are prepended. Order matters. 
             */
            let outer = options.parent
            while (outer) {
                if (r.name) {
                    r.name = outer.name + "." + r.name
                }
                template = outer.match + template
                for (p in outer.params) {
                    r.params[p] = outer.params[p]
                }
                outer = outer.parent
            }

            /*  
                Compile the route and create a RegExp matcher
             */
            if (template is String) {
                r.template = template.replace(/[\(\)]/g, "")
                /*  
                    For string templates, Create a regular expression splitter template so :TOKENS can be referenced
                    positionally in the override hash via $N args.
                    Allow () expressions, these are made into non-capturing parentheses.
                 */
                if (template.contains("(")) {
                    template = template.replace(/\(/g, "(?:")
                    template = template.replace(/\)/g, ")?")
                } else {
                /*  MOB -- should be using AcceptContent
                    if (!template.contains(".:format")) {
                        template += "(.:format)*"
                    }
                 */
                }
                let tokens = template.match(/\{([^\}]+)\}/g)
                for (i in tokens) {
                    tokens[i] = tokens[i].trimStart('{').trimEnd('}')
                }
                let pattern = template.replace(/\{([^\}]+)\}/g, "([^/]*)").replace(/\//g, "\\/")
                r.matcher = RegExp("^" + pattern + "$")
                /*  Splitter ends up looking like "$1:$2:$3:$4" */
                count = 1
                let splitter
                if (!r.spitter) {
                    splitter = ""
                    for (c in tokens) {
                        splitter += "$" + count++ + ":"
                    }
                    r.splitter = splitter.trim(":")
                }
                if (!r.tokens) {
                    r.tokens = tokens
                }
            } else {
                r.template = template
                if (template is Function) {
                    r.matcher = template
                } else if (template is RegExp) {
                    r.matcher = template
                } else if (template) {
                    r.matcher = RegExp(template.toString())
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
            let action = params.action
            if (action) {
                if (action.contains(/[\.\/]/)) {
                    let [controller, act] = action.trimStart("/").split(/[\.\/]/)
                    params.action = action = act || "index"
                    params.controller = controller
                } 
                if (action.contains("::")) {
                    let [ns, act] = action.split("::")
                    params.action = action
                    params.namespace = ns
                }
/*
            } else if (!tokens[action]) {
                params.action = name
*/
            }
            if (options.controller) {
                params.controller = options.controller
            }
            for (field in params) {
                let value = params[field]
                if (value.contains("{")) {
                    let ptokens = value.match(/([^\}*]\})/g)
                    count = 1
                    let splitter = ""
                    for (c in ptokens) {
                        splitter += "$" + count++ + ":"
                    }
                    splitter = splitter.trim(":")
//MOB use different field names
                    params[field] = {tokens: ptokens, splitter: splitter}
                }
            }
            r.run = options.run || lookupRunners(r)
            if (!(r.run is Function)) {
                r.run = function(request) {
                    return options.run
                }
            }
            r.limits = options.limits
            r.linker = options.linker
            r.location = options.location
            r.method = options.method
            r.module = options.module
            r.rewrite = options.rewrite
            r.redirect = options.redirect
            r.threaded = options.threaded
            r.trace = options.trace

            if (options.parent) {
                /* Must process nested routes first before appending the parent route to the routes table */
                //  MOB -- needs work
                options.parent.parent = r
                addRoutes(r.subroute, r)
            }
            routeLookup[r.name] = r

//MOB functionalize
            let inserted
            if (options.first) {
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
            } else {
                for (let [i, route] in routes) {
                    if (route.name == r.name) {
                        routes.remove(i, i)
                        inserted = routes.insert(i, r)
                        break
                    }
                }
            }
            if (!inserted) {
                routes.append(r)
            }
            let resource = options.resource || ""
            let map = resources[resource] ||= {}
            map[name] = r
        }

        public function lookup(options: Object): Route {
            if (options is String) {
                options = {route: options}
            }
            let resource = options.resource || ""
            let map = resources[resource]
            let routeName = options.route || (options.resource ? (options.resource + "-default") : "default")
            return map[routeName]
        }

        public function replace(name: String, template, options: Object = {}): Void
            add(name, template, options)

        public function remove(name: String): Void {
            for (i in routes) {
                if (routes[i].name == name) {
                    routes.remove(i)
                    break
                }
            }
        }

        /** 
            Route a request. The request is matched against the configured route table. 
            The call returns the web application to execute.
            @param request The current request object
            @return The web application function of the signature: 
                function app(request: Request): Object
         */
        public function route(request): Function {
            let params = request.params
//XX dump("ROUTE PARAMS", params)
            let pathInfo = request.pathInfo
            let log = request.log
            log.debug(5, "Routing " + request.pathInfo)

//XX print("REQ " + request.method + " PATH " + pathInfo)
            if (request.method == "POST") {
                let method = request.params["-ejs-method-"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method && method.toUpperCase() != request.method) {
                    // MOB automatically done request.originalMethod ||= request.method
                    log.debug(3, "Change method from " + request.method + " TO " + method + " for " + request.uri)
                    request.method = method
//XX print("MAP method to " + method)
                }
            }

            //  MOB - need better way to turn on debug trace without slowing down the router
            for each (r in routes) {
                log.debug(6, "Test route \"" + r.name + "\"")

                if (r.method) {
                    if (!request.method.contains(r.method)) {
                        continue
                    }
                }
//XX print("@@@@ Test route \"" + r.name + " MATCH \"" + pathInfo + "\".match(" + r.matcher + ")")
                if (r.matcher is Function) { 
                    if (!r.matcher(request)) {
                        continue
                    }

                } else if (!r.splitter) { 
                    if (r.matcher) {
                        let results = pathInfo.match(r.matcher)
                        if (!results) {
                            continue
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
                        params[r.tokens[i]] ||= parts[i].trimStart("/")
                    }
                }
                /*  Apply override params */
                for (field in r.params) {
                    let value = r.params[field]
                    if (value.contains("$") && !r.splitter) {
                        //  MOB -- not right
                        value = pathInfo.replace(r.matcher, value)
                    }
                    if (value.contains("{")) {
                        //  MOB -- not right
                        value = request[value.trim.slice(1,-1)]
                    }
                    params[field] = value
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
                if (r.module && !r.initialized) {
                    global.load(r.module + ".mod")
                    r.initialized = true
                }
//XX print("@@@@ Matched route \"" + r.name + "\"")
//XX dump("PARAMS", request.params)
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
//  MOB -- what if run is a response hash?
                let app = r.run(request)

                if (app == null) {
                    return function(request) {}
                }
                return app
            }
            throw "No route for " + pathInfo
        }

        public function show(all: Boolean = false): Void {
            print("Route Table:")
            for each (r in routes) {
                let method = r.method || "ALL"
                let target, params = r.params, tokens = r.tokens
                if (params.controller || params.action || 
                        tokens && (tokens.contains("action") || tokens.contains("controller"))) {
                    let controller = params.controller || "*"
                    let action = params.action || "*"
                    target = controller + "." + action
                } else if (r.run) {
                    target = r.run.name
                } else {
                    target = "UNKNOWN"
                }
                let template = r.template
                if (template is String) {
                    template = "%s  " + template
                } else if (template is RegExp) {
                    template = "%r  " + template
                } else if (template is Function) {
                    template = "%f  " + template.name
                } else if (!template) {
                    template = "*"
                }
                let line = "%16s: %20s: %7s:  %s".format(r.name, target, method, template)
                if (all) {
                    if (params && Object.getOwnPropertyCount(params) > 0) {
                        if (!(params.action && Object.getOwnPropertyCount(params) == 1)) {
                            line += "  %s".format(serialize(params))
                        }
                    }
                    line += "\n                                             matcher: " + r.matcher + "\n"
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

        If the URL template is a regular expression, it is used to match against the request pathInfo. If it matches,
        the pathInfo is matched and sub-expressions may be referenced in the override parameters by using $1, $2 and
        so on. e.g. { priority: "$1" }
        
        If the URL template is a function, it is run to test for a request match. It should return true to 
        accept the request. The function can set parameters in request.params.

        The optional override hash provides parameters which will be defined in params[] overriding any tokenized 
        parameters previously set.
      
        Routes are tested in-order from first to last. Inner routes are tested before their outer parent.
     */
    enumerable dynamic class Route {
        use default namespace public

        /**
            Template pattern for URIs. The template is used to match the request pathInfo. The template can be a 
            uri-template string, a regular expression or a function. If it is a string, it may contain tokens enclosed 
            in {} and is converted to a regular expression. The tokens are extracted and mapped to items in the 
            Request.params collection. ie. params[NAME]. 

            If the template is a regular expression, it is used to match against the request pathInfo. If it matches
            the pathInfo, then sub-expressions may be referenced in the $params values by using $1, $2 and
            so on. e.g. params: { priority: "$1" }
            
            If the template is a function, it is run to test for a request match. The function should return true to 
            match the request. The function can directly set parameters in request.params.
        */
        var template: Object

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
            Resource limits for the request. See HttpServer.limits for details.
         */
        var limits: Object

        /**
            Function that will be called by Request.link to make to make URI links.

                function linker(uri: Uri, request: Request, options: Object): Uri
         */
        var linker: Function

        /**
            Application location to serve the request. Location contains two properties: prefix which is the string 
            URI prefix for the application and dir which is a Path to the physical file system directory containing 
            the MVC applciation.
         */
        var location: Object

        /**
            Name of a required module containing code to serve requests on this route.  
         */
        var moduleName: String

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Rewrite function to rewrite the request before serving. It may update the request scriptName, pathInfo 
            and other Request properties.
         */
        var rewrite: Function

        /**
            URI to redirect the request toward.
         */
        var redirect: String

        /** 
          Router instance reference
         */
        var router: Router

        /**
            Function to run to serve the request.
         */
        var runner: Function

        /**
            Nested route. A nested route prepends the outer route template to its template. 
            The param set of the outer route are appended to the inner route.
         */
        var subroute: Route

        /**
            If true, the request should be run in a worker thread if possible. This thread will not be dedicated, 
            but will be assigned as the request requires CPU resources.
         */
        var threaded: Boolean

        /**
            Key tokens in the route template
         */
        var tokens: Array

        /**
            Trace options for the request. Note: the route is created after the Request object is created so the tracing 
            of the connections and request headers will be controlled by the owning server. 
            See HttpServer.trace for trace property fields.
         */
        var trace: Object

        /*
            Matcher function or regular expression. This matches the pathInfo for the route.
         */
        internal var matcher: Object

        /*
            Splitter. This is used as the replacement argument to extract tokens from the pathInfo
         */
        internal var splitter: String

        function Route(router: Router) {
            this.router = router
        }

        /**
            Complete a URI link by adding route token information. 
            @param uri Current URI to complete
            @param request Current request object
            @param options Object hash of options describing the target link. See $Request.link.
            @option resource Name of the RESTFul resource owning the route.
            @option route Name of the route
            @return A Uri object.
         */
        public function completeLink(uri: Uri, request: Request, options: Object): Uri {
            if (options.linker) {
                return options.linker(uri, request, options)
            }
            let resource = options.resource || ""
            let map = router.resources[resource]
            if (!map) {
                throw new ReferenceError("Unknown route resource \"" + resource + "\"")
            }
            let routeName = options.route || (resource ? (resource + "-default") : "default")
            let route = map[routeName]
            if (!route) {
                throw new ReferenceError("Unknown route \"" + routeName + "\"")
            }
// print("RESOURCE \"" + resource + "\" template " + template)
            uri.path = Uri.template(route.template, options, {action: "", controller: request.params.controller}).path
// print("RESULT " + uri)
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
