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
        r.add(/\.es$/,  {run: ScriptBuilder})

        //  Add route for directories and use the DirBuilder to run
        r.add(Router.isDir,    {name: "dir", run: DirBuilder})

        //  Add route for RESTful routes and run with the MvcBuilder
        r.addResources("User")

        //  Manually create restful routes
        r.add("/{controller}",           {action: "create", method: "POST"})
        r.add("/{controller}/init",      {action: "init"})
        r.add("/{controller}",           {action: "index"})
        r.add("/{controller}/{id}/edit", {action: "edit"})
        r.add("/{controller}/{id}",      {action: "show"})
        r.add("/{controller}/{id}",      {action: "update", method: "PUT"})
        r.add("/{controller}/{id}",      {action: "destroy", method: "DELETE"})
        r.add("/{controller}(/do/{action})")
        
        //  Add route for upper or lower case "D" or "d". Run the default app: MvcBuilder
        r.add("/[Dd]ash/refresh", "@Dash/refresh")

        //  Add route for an "admin" application. This sets the scriptName to "admin" expects an MVC application to be
        //  located at the directory "myApp"
        r.add("/admin/", {location: { prefix: "/control", dir: "my"})

        //  Rewrite a request for "old.html" to new.html
        r.add("/web/old.html",  {rewrite: function(request) { request.pathInfo = "/web/new.html"}})

        //  Handle a request with a literal response
        r.add("/oldStuff/", {run: {body: "Not found"} })

        //  Handle a request with an inline function
        r.add("/oldStuff/", {run: function(request) { return {body: "Not found"} }})

        //  A custom matching function to match SSL requests
        r.add(function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }, {
                name: "secure", action: "private" 
            }
        })

        //  A matching function that rewrites a request and then continues matching other routes
        r.add(function (request) {
            if (request.uri.startsWith("/old")) {
                request.uri = request.uri.toString().trimStart("/old")
                return false
            }
        })

        //  Set request parameters with values from request
        r.add("/custom", {action: "display", params: { from: "{uri}", transport: "{scheme}" })

        //  Nest matching routes
        let outer = r.add("/blog", {action: "/post/index"})
        r.add("/comment", {action: "/comment/{action}/{id}", outer: outer})

        //  Match with regular expression. The sub-match is available via $N parameters
        r.add(/^\/Dash-((Mini)|(Full))$/, {controller: "post", action: "list", kind: "$1"})
        
        //  Conditional matching. Surround optional tokens in "()"
        r.add("/Dash(/{product}(/{branch}(/{configuration})))", {   
            name: "dash", 
            method: "POST", 
            controller: "Dash", 
            action: "index",
        })

        //  Replace the home page route
        r.addHome("/Status/")

        //  Display the route table to the console
        r.show()

        @stability prototype
        @spec ejs
     */
    class Router {

        /**
            Symbolic constant for Router() to add top-level routes for directory, *.es, *.ejs and a catchall 
            route for static pages. 
            Use of this constant will not add routes for MVC content or RESTful resources. 
         */ 
        public static const Default = "default"

        /**
            Symbolic constant for Router() to add top-level routes for directory, *.es, *.ejs, generic routes for
            RESTful resources and a catchall route for static pages
         */ 
        public static const Restful = "restful"

        /*
            Routes indexed by first component of the URI path/template
         */
        public var routes: Object = {}
        
        /*
            Map of run functions based on request extension 
UNUSED
        var builders = {
            "es":   ScriptBuilder,
            "ejs":  TemplateBuilder,
            "html": StaticBuilder,
        }
         */

        /**
            Function to test if the Request.filename is a directory.
            @param request Request object to consider
            @return True if request.filename is a directory
         */
        public static function isDir(request) request.filename.isDir

        /**
            Add a catch-all route for static content
         */
        public function addCatchall(): Void
            add(/^\/.*$/, {name: "catchall", run: StaticBuilder, method: "*"})

        /** 
            Direct routes for MVC apps. These map HTTP methods directly to method names.
        */
        public function addDirect(name: String, options: Object = {}): Void {
            add("/" + name + "/(/{id}(/{action}))")
            let names = options.plural || toPlural(name)
            add("/" + names + "/(/{action})", {contrtoller: name, namespace: "GROUP"})
        }

        /**
            Add a home page route. This will add or update the "home" page route.
         */
        public function addHome(target: String): Void
            add("/", { name: "home", action: target})

        /**
            Add restful routes for a singleton resource. 
            Supports member CRUD actions: edit, show, update, and custom actions.
            The restful routes defined are:
            <pre>
                Method  URL                   Action
                GET     /controller/edit      edit        Display a resource form suitable for editing
                GET     /controller           show        Display a resource (not editable)
                PUT     /controller           update      Update a resource (idempotent)
                ANY     /controllers/action   *           Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
            @param name Name of the resource to route. Can also be an array of names.
         */
        public function addResource(name: *): Void {
            if (name is Array) {
                for each (n in name) {
                    addResource(n)
                }
                return
            } 
            add('/' + name + "/edit",      {controller: name, action: "edit"})
            add('/' + name,                {controller: name, action: "show"})
            add('/' + name,                {controller: name, action: "update", method: "PUT"})
            add('/' + name + "/{action}",  {controller: name, name: "default",  method: "*"})
        }

        /** 
            Add restful routes for a resource collection. 
            Supports CRUD actions: edit, index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET     /controller             index       Display an overview of the resource
                GET     /controller/init        init        Initialize and display a blank form for a new resource
                POST    /controller             create      Accept a form creating a new resource

                GET     /controller/1/edit      edit        Display a resource form suitable for editing
                GET     /controller/1           show        Display a resource (not editable)
                PUT     /controller/1           update      Update a resource (idempotent)
                DELETE  /controller/1           destroy     Destroy a resource (idempotent)

                ANY     /controller/action      default     Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
            @param name Name of the resource to route. Can also be an array of names.
        */
        public function addResources(name: *): Void {
            if (name is Array) {
                for each (n in name) {
                    addResources(n)
                }
                return
            } 
            add('/' + name + "/init",       {controller: name, action: "init"})
            add('/' + name,                 {controller: name, action: "index"})
            add('/' + name,                 {controller: name, action: "create", method: "POST"})

            let id = {id: "[0-9]+"}
            add('/' + name + "/{id}/edit",  {controller: name, action: "edit", constraints: id})
            add('/' + name + "/{id}",       {controller: name, action: "show", constraints: id})
            add('/' + name + "/{id}",       {controller: name, action: "update", , constraints: id, method: "PUT"})
            add('/' + name + "/{id}",       {controller: name, action: "destroy", , constraints: id, method: "DELETE"})

            add('/' + name + "/{action}",   {controller: name, name: "default", method: "*"})
        }

        /** 
            Add default restful routes for resources. This adds default routes for generic resources.
            Supports CRUD actions: edit, index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET     /controller             index       Display an overview of the resource
                GET     /controller/init        init        Initialize and display a blank form for a new resource
                POST    /controller             create      Accept a form creating a new resource

                GET     /controller/1/edit      edit        Display a resource form suitable for editing
                GET     /controller/1           show        Display a resource (not editable)
                PUT     /controller/1           update      Update a resource (idempotent)
                DELETE  /controller/1           destroy     Destroy a resource (idempotent)

                ANY     /controller/action      default     Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
        */
        public function addRestful(): Void {
            add("/{controller}/init",               {action: "init"})
            add("/{controller}",                    {action: "index"})
            add("/{controller}",                    {action: "create", method: "POST"})

            let id = {id: "[0-9]+"}
            add("/{controller}/{id}/edit",          {action: "edit", constraints: id})
            add("/{controller}/{id}",               {action: "show", constraints: id})
            add("/{controller}/{id}",               {action: "update", constraints: id, method: "PUT"})
            add("/{controller}/{id}",               {action: "destroy", constraints: id, method: "DELETE"})

            add("/{controller}(/{action})",         {name: "default", method: "*"})
        }

        /**
            Add a default MVC controller/action route. This consists of a "/{controller}/{action}" route.
            All HTTP method verbs are supported.
         */
        public function addDefault(): Void
            add("/{controller}(/{action})", {name: "default", method: "*"})

        /**
            Add routes to handle static content, directories, "es" scripts and stand-alone ejs templated pages.
         */
        public function addHandlers(): Void {
            let staticPattern = "\/" + (App.config.directories.static || "static") + "\/.*"
            if (staticPattern) {
                add(staticPattern, {name: "static", run: StaticBuilder})
            }
            add(/\.es$/,  {name: "es",  run: ScriptBuilder, method: "*"})
            add(/\.ejs$/, {name: "ejs", module: "ejs.template", run: TemplateBuilder, method: "*"})
            add(isDir,    {name: "dir", run: DirBuilder})
        }

        /**
            Create a Router instance
            @param routeSet Name of the route set to add. Supports sets include:
                Router.Default, Router.Direct, Router.Handlers, Router.Restful
         */
        function Router(routeSet: String = null) {
            switch (routeSet) {
            case Default:
                addHandlers()
                addCatchall()
                break
            case Restful:
                addHandlers()
                addRestful()
                addCatchall()
                break
            case null:
                break
            default:
                throw "Unknown route set: " + name
            }
        }

        /** 
UNUSED
            Add a builder function for an extension
            @param builder Builder function. Builders must be of the form
                function builder(request: Request): Function
            @param ext 
        function addBuilder(builder: Function, ext: String): Void
            builders[ext] = builder

        private function lookupBuilders(ext): Function
            builders[ext] || MvcBuilder
         */

        private function insertRoute(r: Route, options: Object): Void {
            let routeSetName
            if (r.template is String) {
                routeSetName = r.template.split("{")[0].split("/")[1]
            }
            r.routeSetName = routeSetName || ""
            let routeSet = routes[r.routeSetName] ||= {}
            routeSet[r.name] = r
        }

        /*
            Add a route
            @param template String or Regular Expression defining the form of a matching URI (Request.pathInfo).
            @param options Route options representing the URI and options to use when servicing the request. If it
                is a string, it may begin with an "@" and be of the form "@[controller/]action". In this case, if there
                is a "/" delimiter, the first portion is a controller and the second is the controller action to invoke.
                The controller or action may be absent. For example: "@Controller/", "@action", "@controller/action".
                If the string does not begin with an "@", it is interpreted as a literal URI. For example: "/web/index.html".
                If the options is an object hash, it may contain the options below:
            @option action Action method to service the request. This may be of the form "controller/action" or "controller/"
            @option controller Controller to service the request.
            @option name Name to give to the route. If absent, the name is created from the controller and action names.
            @option outer Parent route. The parent's template and parameters are appended to this route.
            @option params Override parameter to provide to the request in the Request.params.
            @examples:
                Route("/{controller}(/{action}(/{id}))/", { method: "POST" })
                Route("/User/login", {name: "login" })
            @option name Name for the route
            @option method String|RegExp HTTP methods to support.
            @option limits Limits object for the requests on this route. See HttpServer.limits.
            @option location Object hash with properties prefix and dir
            @option params Override request parameters.
            @option parent Outer parent route
            @option redirect Redirect requests on this route to this URI.
            @option rewrite Rewrite function. This can rewrite request properties.
            @option run (Function|Object) This can be either a builder function to serve the request or it can be a 
                response hash with status, headers and body properties. The builder function should return a function 
                of the form:
                    function (request: Request): Object
            @example:
                r.add("/User/{action}", {controller: "User"})
         */
        public function add(template: Object, options: Object = null): Route {
            let r = new Route(template, options, this)
            insertRoute(r, options)
            return r
        }

        /*
            Lookup a route
            @param options Route description. This can be either string or object hash. If it is a string, it should be
                of the form "controller/action". If the options is an object hash, it should have properties
                controller and action. The controller is used as the index for the route set. The action property is
                the index for the route name.
            @return Route object
         */
        public function lookup(options: Object): Route {
            if (options is String) {
                if (options[0] == "@") {
                    options = options.slice(1)
                }
                if (options.contains("/")) {
                    let [controller, action] = options.split("/")
                    let routeSet = routes[controller]
                    if (routeSet) {
                        return routeSet[action]
                    }
                }
                return routes[""][options]
            }
            let controller = options.controller || ""
            let routeSet = routes[controller]
            let routeName = options.route || options.action || "default"
            return routeSet[routeName]
        }

        /**
            Replace a route route
            @param template String or Regular Expression defining the form of a matching URI (Request.pathInfo).
            @param options Route options representing the URI and options to use when servicing the request. If it
                is a string, it may begin with an "@" and be of the form "@[controller/]action". In this case, if there
                is a "/" delimiter, the first portion is a controller and the second is the controller action to invoke.
                The controller or action may be absent. For example: "@Controller/", "@action", "@controller/action".
                If the string does not begin with an "@", it is interpreted as a literal URI. For example: "/web/index.html".
                If the options is an object hash, it may contain the options below:
                See $add() for option details.
         */
        public function replace(template, options: Object = {}): Void
            add(template, options)

        /*
            Remove a route
            @param name Name of the route to remove. Name should be of the form "controller/action" where controller
            is the index for the route set and action is the index for the route name.
         */
        public function remove(name: String): Void {
            let routeSet = routes[name.split("/")[0]]
            for (let routeName in routeSet) {
                if (routeName == name) {
                    delete routeSet[route]
                    break
                }
            }
        }

        /*
            Make the application function to service the request
         */
        private function makeApp(request: Request, r: Route): Function {
            let params = request.params
            let pathInfo = request.pathInfo
            let log = request.log

            for (field in r.params) {
                /*  Apply override params */
                let value = r.params[field]
                if (value.toString().contains("$")) {
                    value = pathInfo.replace(r.pattern, value)
                }
                if (value.toString().contains("{")) {
                    value = Uri.template(value, params, request)
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
            if (log.level >= 3) {
                log.debug(3, "Matched route \"" + r.routeSetName + "/" + r.name + "\"")
                if (log.level >= 5) {
                    log.debug(5, "  Route params " + serialize(params, {pretty: true}))
                }
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
            return r.builder(request)
        }

        /** 
            Route a request. The request is matched against the configured route table. 
            The call returns the web application to execute.
            @param request The current request object
            @return The web application function of the signature: 
                function app(request: Request): Object
         */
        public function route(request): Function {
            let log = request.log
            log.debug(5, "Routing " + request.pathInfo)

            if (request.method == "POST") {
                let method = request.params["-ejs-method-"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method && method.toUpperCase() != request.method) {
                    log.debug(3, "Change method from " + request.method + " TO " + method + " for " + request.uri)
                    request.method = method
                }
            }
            let routeSet = routes[request.pathInfo.split("/")[1]]
            for each (r in routeSet) {
                log.debug(5, "Test route \"" + r.name + "\"")
                if (r.match(request)) {
                    log.debug(3, "Match route \"" + r.name + "\"")
                    return makeApp(request, r)
                }
            }
            routeSet = routes[""]
            for each (r in routeSet) {
                log.debug(5, "Test route \"" + r.name + "\"")
                if (r.match(request)) {
                    log.debug(3, "Match route \"" + r.name + "\"")
                    return makeApp(request, r)
                }
            }
            throw "No route for " + request.pathInfo
        }

        /**
            Show the route table
            @param extra Set to true to display extra route information
         */
        public function show(extra: Boolean = false): Void {
            let lastController
            for each (name in Object.getOwnPropertyNames(routes).sort()) {
                print("\n" + (name || "Global")+ ":")
                for each (r in routes[name]) {
                    showRoute(r, extra)
                }
            }
            print()
        }

        private function showRoute(r: Route, extra: Boolean = false): Void {
            let method = r.method || "*"
            let target
            let tokens = r.tokens
            let params = r.params || {}
            if (params.controller || params.action || 
                    (tokens && (tokens.contains("action") || tokens.contains("controller")))) {
                let controller = params.controller || "*"
                let action = params.action || "*"
                target = controller + "/" + action
            } else if (r.response) {
                target = "Response Object"
            } else if (r.builder) {
                target = r.builder.name
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
            let line = "  %-24s %-24s %-7s %s".format(r.name, target, method, template)
            if (extra) {
                if (params && Object.getOwnPropertyCount(params) > 0) {
                    if (!(params.action && Object.getOwnPropertyCount(params) == 1)) {
                        line += "\n                                                    %s".format(serialize(params))
                    }
                }
                line += "\n                                                    pattern: " + r.pattern + "\n"
            }
            print(line)
        }
    }

    /** 
        Route class. A Route describes a mapping from a URI to a web application. A route has a URI template and a
        serving function.

        If the URI template is a regular expression, it is used to match against the request pathInfo. If it matches,
        the pathInfo is matched and sub-expressions may be referenced in the override parameters by using $1, $2 and
        so on. e.g. { priority: "$1" }
        
        If the URL template is a function, it is run to test for a request match. It should return true to 
        accept the request. The function can set parameters in request.params.

        The optional override params hash provides parameters which will be defined in params[] overriding any tokenized 
        parameters previously set.
     */
    enumerable dynamic class Route {
        use default namespace public

        /* 
            Seed for generating route names 
         */
        private static var nameSeed: Number = 0

        /**
            Builder function to create a function app to serve the request. The builder should return return a function 
            that will be invoked serve the request. The builder can also be a response object hash.
         */
        var builder: Object

        /**
            Resource limits for the request. See HttpServer.limits for details.
         */
        var limits: Object

        /**
            Application location to serve the request. Location contains two properties: prefix which is the string 
            URI prefix for the application and dir which is a Path to the physical file system directory containing 
            the MVC applciation.
         */
        var location: Object

        /**
            HTTP method to match. If set to "" or "*", all methods are matched.
         */
        var method: String

        /**
            Middleware to run on requests for this route. Middleware wraps the application function filtering and 
            modifying its inputs and outputs.
         */
        var middleware: Array

        /**
            Route name. This is local to the route set (controller)
         */
        var name: String

        /**
            Name of a required module containing code to serve requests on this route.  
         */
        var moduleName: String

        /*
            Original template as supplied by caller
         */
        var originalTemplate: Object

        /**
            Outer route for a nested route. A nested route prepends the outer route template to its template. 
            The param set of the outer route is appended to the inner route.
         */
        var outer: Route

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Rewrite function to rewrite the request before serving. It may update the request scriptName, pathInfo 
            and other Request properties. Return true to continue serving the request on this route. Otherwise re-route
            after rewriting the request. 
         */
        var rewrite: Function

        /**
            URI to redirect the request toward.
         */
        var redirect: String

        /**
            Response object hash
         */
        var response: Object

        /** 
          Router instance reference
         */
        var router: Router

        /**
            Route set owning the route. This is the first component of the template.
         */
        var routeSetName: String

        /**
            Template pattern for URIs. The template is used to match the request pathInfo. The template can be a 
            string, a regular expression or a function. If it is a string, it may contain tokens enclosed in braces 
            "{}" and it is converted to a regular expression for fast matching. At run-time, request tokens 
            are extracted and stored as items in the Request.params collection.

            If the template is a regular expression, it is used to match against the request pathInfo. If it matches, 
            then sub-expressions may be referenced in the $params values by using $1, $2 and so on. 
            e.g. params: { priority: "$1" }
            
            If the template is a function, it is run to test for a request match. The function should return true to 
            match the request. The function can directly set parameters in request.params.
        */
        var template: Object

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
            Match function
         */
        internal var match: Function

        /*
            Regular expression pattern. This matches the pathInfo for the route.
         */
        internal var pattern: Object

        /*
            Splitter. This is used as the replacement argument to extract tokens from the pathInfo
         */
        internal var splitter: String

        /*
            Create a new Route instance. This is normally not invoked directly. Rather Router.add() is used to
            create and install routes into the Router.
            @param template String or Regular Expression defining the form of a matching URI (Request.pathInfo).
            @param options Route options representing the URI and options to use when servicing the request. If it
                is a string, it may begin with an "@" and be of the form "@[controller/]action". In this case, if there
                is a "/" delimiter, the first portion is a controller and the second is the controller action to invoke.
                The controller or action may be absent. For example: "@Controller/", "@action", "@controller/action".
                If the string does not begin with an "@", it is interpreted as a literal URI. For example: "/web/index.html".
                If the options is an object hash, it may contain the options below:
            @option action Action method to service the request. This may be of the form "controller/action" or "controller/"
            @option controller Controller to service the request.
            @option name Name to give to the route. If absent, the name is created from the controller and action names.
            @option outer Parent route. The parent's template and parameters are appended to this route.
            @option params Override parameter to provide to the request in the Request.params.
            @examples:
                Route("/{controller}(/{action}(/{id}))/", { method: "POST" })
                Route("/User/login", {name: "login" })
         */
        function Route(template: Object, options: Object, router: Router) {
            this.router = router
            this.template = template
            options = parseOptions(options)
            makeParams(options)
            inheritRoutes(options)
            compileTemplate(options)
            setName(options)
            setRouteProperties(options)
        }

        /**
            Get the template pattern for a route given a controller and a route name. If the specified controller 
            cannot be found, the Global route set is used. If the specified route name cannot be found, the "default"
            route is used. Use Uri.template to expand the template with URI components.
            @param controller Controller name
            @param routeName Route name to look for
            @return A template URI string
         */
        public function getTemplate(controller: String, routeName: String): String {
            let routes = router.routes
            let routeSet = routes[controller] || routes[""]
            let route = routeSet[routeName] || routeSet["default"] || routes[""]["default"]
            return "/{scriptName}" + route.template
        }

        private function inheritRoutes(options: Object): Void {
            let parent = options.outer
            if (parent) {
                let ptem = (parent.originalTemplate is RegExp) ? parent.originalTemplate.source : parent.originalTemplate
                let tem = (template is RegExp) ? template.source : template
                if (template is RegExp) {
                    template = RegExp(ptem + tem)
                } else {
                    template = ptem + tem
                }
                for (p in parent.params) {
                    params[p] = parent.params[p]
                }
            }
            this.originalTemplate = template
        }

        private function compileTemplate(options: Object): Void {
            if (template is String) {
                let t = template
                /*  
                    For string templates, Create a regular expression splitter template so :TOKENS can be referenced
                    positionally in the override hash via $N args.
                    Allow () expressions, these are made into non-capturing parentheses.
                 */
                if (t.contains("(")) {
                    t = t.replace(/\(/g, "(?:")
                    t = t.replace(/\)/g, ")?")
                }
                tokens = t.match(/\{([^\}]+)\}/g)
                for (i in tokens) {
                    tokens[i] = tokens[i].trimStart('{').trimEnd('}')
                }
                let constraints = options.constraints
                for each (token in tokens) {
                    if (constraints && constraints[token]) {
                        t = t.replace("{" + token + "}", "(" + constraints[token] + ")")
                    } else {
                        t = t.replace("{" + token + "}", "([^/]*)")
                    }
                } 
                t = t.replace(/\//g, "\\/")
                pattern = RegExp("^" + t + "$")
                /*  Splitter ends up looking like "$1:$2:$3:$4" */
                let count = 1
                if (!splitter) {
                    splitter = ""
                    for (c in tokens) {
                        splitter += "$" + count++ + ":"
                    }
                    splitter = splitter.trim(":")
                }
                match = matchAndSplit
                template = template.replace(/[\(\)]/g, "")
            } else {
                if (template is Function) {
                    match = template
                } else if (template is RegExp) {
                    pattern = template
                    match = matchRegExp
                } else if (template) {
                    pattern = RegExp(template.toString())
                    match = matchRegExp
                }
            }
        }

        /*
            Match a request and apply splitter to create request params
         */
        private function matchAndSplit(request: Request): Boolean {
            if (method && !request.method.contains(method)) {
                return false
            }
            let pathInfo = request.pathInfo
            if (!pathInfo.match(pattern)) {
                return false
            }
            let parts = pathInfo.replace(pattern, splitter).split(":")
            for (i in tokens) {
                request.params[tokens[i]] ||= parts[i].trimStart("/")
            }
            return true
        }

        /*
            Match a request using a regular expression without splitter
         */
        private function matchRegExp(request: Request): Boolean {
            if (method && !request.method.contains(method)) {
                return false
            }
            return request.pathInfo.match(pattern)
        }

        /*
            Make the params object based on input options
         */
        private function makeParams(options: Object): Void {
            params = options.params || {}
            if (options.action) {
                params.action = options.action
            }
            let action = params.action
            if (action) {
                if (action.contains("/")) {
                    let [controller, act] = action.trimStart("/").split("/")
                    params.action = action = act || "index"
                    params.controller = controller
                } 
                if (action.contains("::")) {
                    let [ns, act] = action.split("::")
                    params.action = action
                    params.namespace = ns
                }
            }
            if (options.controller) {
                params.controller = options.controller
            }
        }

        private function parseOptions(options: Object): Object {
            if (!options) {
                let t = template.replace(/[\(\)]/g, "")
                options = {action: "@" + t.split("{")[0].trimStart("/")}
            } else if (options is String) {
                options = {action: options}
            }
            let action = options.action
            if (action) {
                if (action is Function) {
                    options.run ||= options.action
                } else if (action[0] == '@') {
                    [options.controller, options.action] = action.slice(1).split("/")
                }
            }
            if (options.middleware) {
                middleware = options.middleware.reverse()
            }
            return options
        }

        /*
            Create a useful (deterministic) name for the route
         */
        private function setName(options: Object) {
            name = options.name || options.action || "index"
            if (outer && !options.name) {
                name = options.name + "/" + name
            }
            if (!name) {
                throw "Route has no name defined"
            }
        }

        private function setRouteProperties(options: Object): Void {
            limits = options.limits
            linker = options.linker
            location = options.location
            moduleName = options.module
            rewrite = options.rewrite
            redirect = options.redirect
            threaded = options.threaded
            trace = options.trace
            if (options.method == "" || options.method == "*") {
                options.method = ""
            } else {
                method = options.method || "GET"
            }
            if (options.run is Function) {
                builder = options.run || MvcBuilder
            } else if (options.run) {
                response = options.run
                builder = function (request) {
                    return function (request) {
                        return response
                    }
                }
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
