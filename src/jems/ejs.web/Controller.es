/*
    Controller.es -- MVC Controller class.
 */

module ejs.web {

    /* 
        Namespace for all action methods 
     */
    namespace action = "action"

    /** 
        Web framework controller class.
        @stability prototype
        @spec ejs
     */
    enumerable class Controller {
        /*  
            Define properties and functions in the ejs.web namespace so that user controller variables don't clash. 
            Override with "public" the specific properties that must be copied to views.
         */
        use default namespace module

        private static var _initRequest: Request

        private var redirected: Boolean
        private var _afterFilters: Array
        private var _beforeFilters: Array
        private var _wrapFilters: Array
        private var lastFlash: Object

        /** Name of the action being run */
        var actionName:  String 

        /** Configuration settings - reference to Request.config */
        var config: Object 

        /** Lower case controller name */
        var controllerName: String

        /** Deployment mode: debug, test, production */
        var deploymentMode: String

        /** Logger stream - reference to Request.log */
        var log: Logger

        /** Form and query parameters - reference to the Request.params object. */
        var params: Object

        /** The response has been rendered. If an action does not render a response, then a default view will be rendered */
        var rendered: Boolean

        /** Reference to the current Request object */
        var request: Request

        /** Reference to the current View object */
        var view: View

        /** 
            Flash messages to display on the next screen
                "error"         Negative errors (Warnings and errors)
                "inform"        Informational / postitive feedback (note)
                "warn"          Negative feedback (Warnings and errors)
                "*"             Other feedback (reminders, suggestions...)
        */
        public var flash: Object

        /***************************************** Convenience Getters  ***************************************/

        /** @duplicate Request.absHome */
        function get absHome(): Uri 
            request ? request.absHome : null

        /** @duplicate Request.home */
        function get home(): Uri 
            request ? request.home : null

        /** @duplicate Request.pathInfo */
        function get pathInfo(): String 
            request ? request.pathInfo : null

        /** @duplicate Request.session */
        function get session(): Session 
            request ? request.session : null

        /** @duplicate Request.uri */
        function get uri(): Uri 
            request ? request.uri : null

        /********************************************* Methods *******************************************/
        /** 
            Create and initialize a controller. This may be called directly by class constructors or via 
            the Controller.create factory method.
            @param req Web request object
         */
        function Controller(req: Request) {
            //  initRequest may be set by create() to allow subclasses to omit constructors
            request = req || _initRequest
            if (request) {
                log = request.log
                params = request.params
                controllerName = typeOf(this).trim("Controller") || "-DefaultController-"
                config = request.config
                if (config.database) {
                    openDatabase(request)
                }
            }
        }

        /** MOB */
        function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /** MOB */
        function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /** 
            Factory method to create and initialize a controller. The controller class is specified by 
            params["controller"] which should be set by the router to the controller name without the "Controller" suffix. 
            This call expects the controller class to be loaded. Called by Mvc.load().
            @param request Web request object
         */
        static function create(request: Request): Controller {
            let cname: String = request.params["controller"]
            if (!cname) {
                throw "Can't run app, controller " + cname + " is not loaded"
            }
            _initRequest = request
            let uname = cname.toPascal() + "Controller"
            let c: Controller = new global[uname](request)
            c.request = request
            _initRequest = null
            return c
        }

        /** 
            Send an error notification to the user. This is just a convenience instead of setting flash["error"]
            @param msg Message to display
         */
        function error(msg: String): Void {
            flash ||= {}
            flash["error"] = msg
        }

        /** 
            @duplicate Rquest.header
         */
        function header(key: String): String
            request.header(key)

        /** 
            Send a positive notification to the user. This is just a convenience instead of setting flash["inform"]
            @param msg Message to display
         */
        function inform(msg: String): Void {
            flash ||= {}
            flash["inform"] = msg
        }

        /** 
            @duplicate Request.makeUri
            @option controller The name of the controller to use in the URI.
            @option action The name of the action method to use in the URI.
         */
        function makeUri(location: Object): Uri
            request.makeUri(location)

//  MOB - could this use a general meta facility
        /** 
            Missing action method. This method will be called if the requested action routine does not exist.
         */
        action function missing(): Void {
            rendered = true
            throw "Missing Action: \"" + params.action + "\" could not be found for controller \"" + controllerName + "\""
        }

//  MOB -- are there any controller events?
        /** 
            @duplicate Request.observe
         */
        function observe(name, observer: Function): Void
            request.observer(name, observer)

        /** 
            @duplicate Stream.read
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 
            request.read(buffer, offset, count)

        /** 
            Redirect the client to the given URL
            @param where Url to redirect the client toward. This can be a relative or absolute string URL or it can be
                a hash of URL components. For example, the following are valid inputs: "../index.ejs", 
                "http://www.example.com/home.html", {action: "list"}.
            @param status Http status code to use in the redirection response. Defaults to 302.
         */
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void {
            request.redirect(where, status)
            redirected = true
        }

        /** 
            Redirect the client to the given action
            @param action Controller action name to which to redirect the client.
         */
        function redirectAction(action: String): Void
            redirect({action: action})

        /** 
            Render the raw arguments back to the client. The args are converted to strings.
            @param args Arguments to write to the client
         */
        function render(...args): Void { 
            rendered = true
            request.write(args)
            request.finalize()
        }

        /**
            Render an error message as the response
         */
        function renderError(status: Number, ...msgs): Void {
            rendered = true
            request.writeError(status, ...msgs)
        }

        /** 
            Render a file's contents. 
            @param filename Path to the filename to send to the client
         */
        function renderFile(filename: Path): Void { 
            rendered = true
            request.sendFile(filename)
            request.finalize()
        }

        /** 
            Render a partial ejs template. Does not set "rendered" to true.
         */
        function renderPartial(path: Path): void { 
            //  MOB -- todo
        }

        /** 
            Render a view template
         */
        function renderView(viewName: String? = null): Void {
            if (rendered) {
                throw new Error("renderView invoked but render has already been called")
                return
            }
            viewName ||= actionName
            let viewClass = controllerName + "_" + viewName + "View"
            loadView(viewName)
            view = new global[viewClass](request)
            view.controller = this
            //  MOB -- slow. Native method for this?
            for each (let n: String in Object.getOwnPropertyNames(this, {includeBases: true, excludeFunctions: true})) {
                view.public::[n] = this[n]
            }
            log.debug(4, "render view: \"" + controllerName + "/" + viewName + "\"")
            rendered = true
            view.render(request)
        }

        /** 
            Run the controller action. 
            @param request Request object
            @return A response object hash {status, headers, body} or null if writing directly using the request object.
         */
//  MOB -- is this a builder or what?
        function run(request: Request): Object {
            actionName = params.action || "index"
            params.action = actionName
            use namespace action
            if (request.sessionID) {
                flashBefore()
            }
            runFilters(_beforeFilters)
            let response
            if (!redirected) {
                if (!this[actionName]) {
                    if (!viewExists(actionName)) {
                        actionName = "missing"
                        response = this[actionName]()
                    }
                } else {
                    response = this[actionName]()
                }
                if (!response && !rendered && !redirected && request.autoFinalize) {
                    /* Run a default view */
                    renderView()
                }
                runFilters(_afterFilters)
            }
            if (flash) {
                flashAfter()
            }
            if (!response) {
                request.finalize()
            }
            return response
        }

        /** MOB */
        function resetFilters(): Void {
            _beforeFilters = null
            _afterFilters = null
            _wrapFilters = null
        }

        /** @duplicate Request.setHeader */
        function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /** @duplicate Request.setStatus */
        function setStatus(status: Number): Void
            request.status = status

        /** 
            Send a warning message back to the client for display in the flash area. This is just a convenience instead of
            setting flash["warn"]
            @param msg Message to display
         */
        function warn(msg: String): Void {
            flash ||= {}
            flash["warn"] = msg
        }

        /** MOB */
        function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        /** 
            Low-level write data to the client. This will buffer the written data until either flush() or 
            finalize() is called.  This will not set the $rendered property.
            @duplicate Request.write
         */
        function write(...data): Number
            request.write(...data)

        /**************************************** Private ******************************************/
        /* 
            Save the flash message for the next request. Delete old flash messages
         */
        private function flashAfter() {
            if (lastFlash) {
                for (item in flash) {
                    for each (old in lastFlash) {
                        if (hashcode(flash[item]) == hashcode(old)) {
                            delete flash[item]
                        }
                    }
                }
            }
//  MOB -- obj.length was so much easier!
            if (Object.getOwnPropertyCount(flash) > 0) {
                request.session["__flash__"] = flash
            }
        }

        /* 
            Prepare the flash message. This extracts any flash message from the session state store
         */
        private function flashBefore() {
            lastFlash = null
            flash = request.session["__flash__"]
            if (flash) {
                request.session["__flash__"] = undefined
                lastFlash = flash.clone()
            }
        }

        /**
            Load the view. 
            @param viewName Bare view name
            @hide
         */
        private function loadView(viewName: String) {
            let dirs = config.directories
            let cvname = controllerName + "_" + viewName
            let path = request.dir.join("views", controllerName, viewName).joinExt(config.extensions.ejs)
            let cached = Loader.cached(path, request.config, request.dir.join(dirs.cache))
            let viewClass = cvname + "View"

//  MOB -- can this be generalized and use the Web.serve code?
            //  TODO - OPT. Could keep a cache of cached.modified
            if (global[viewClass] && cached.modified >= path.modified) {
                log.debug(4, "Use loaded view: \"" + controllerName + "/" + viewName + "\"")
                return
            } else if (!path.exists) {
                throw "Missing view: \"" + path+ "\""
            }
            if (cached && cached.exists && cached.modified >= path.modified) {
                log.debug(4, "Load view \"" + controllerName + "/" + viewName + "\" from: " + cached);
                load(cached)
            } else {
                if (!global.TemplateParser) {
                    load("ejs.web.template.mod")
                }
                let layouts = request.dir.join(dirs.layouts)
                log.debug(4, "Rebuild view \"" + controllerName + "/" + viewName + "\" and save to: " + cached);
                if (!path.exists) {
                    throw "Can't find view: \"" + path + "\""
                }
                let code = TemplateParser().buildView(cvname, path.readString(), { layouts: layouts })
                eval(code, cached)
            }
        }

        /*
            Generic open of a database. Expects and ejscr configuration like:

            mode: "debug"
            database: {
                class: "Database",
                adapter: "sqlite3",
                debug: {
                    name: "db/blog.sdb", trace: true, 
                }
            }
         */
        private function openDatabase(request: Request) {
            let deploymentMode = config.mode
            let dbconfig = config.database
            let klass = dbconfig["class"]
            let adapter = dbconfig.adapter
            let profile = dbconfig[deploymentMode]
            if (klass && dbconfig.adapter && profile.name) {
                //  MOB -- should NS be here
                use namespace "ejs.db"
                let db = new global[klass](dbconfig.adapter, request.dir.join(profile.name))
                if (profile.trace) {
                    db.trace(true)
                }
            }
        }

        /* 
            Run the before/after filters. These are typically used to handle authorization and similar tasks
         */
        private function runFilters(filters: Array): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    only = options.only
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
                }
                fn.call(this)
            }
        }

        private function viewExists(name: String): Boolean {
            let viewClass = controllerName + "_" + actionName + "View"
            if (global[viewClass]) {
                return true
            }
            let path = request.dir.join("views", controllerName, name).joinExt(config.extensions.ejs)
            if (path.exists) {
                return true
            }
            return null
        }

        /********************************************  LEGACY 1.0.2 ****************************************/

        /** @hide
            @deprecated
         */
        function get appUrl()
            request.home.toString().trimEnd("/")

        /** @hide
            @deprecated
         */
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String
            makeUri({ path: action })
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
