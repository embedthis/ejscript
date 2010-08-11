/*
    Controller.es -- MVC Controller class.
 */

module ejs.web {
    /**
        Namespace for all action methods 
     */
    namespace action = "action"

    /** 
        Web framework controller class. The controller classes can accept web requests and direct them to action methods
        which generate the response. Controllers are responsible for either generating output for the client or invoking
        a View which will create the response.
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

        /** Name of the action being run */
        var actionName:  String 

        /** Configuration settings - reference to Request.config */
        var config: Object 

        /** Lower case controller name */
        var controllerName: String

        /** Logger stream - reference to Request.log */
        var log: Logger

        /** Form and query parameters - reference to the Request.params object. */
        var params: Object

        /** The response has been rendered. If an action does not render a response, then a default view will be rendered */
        var rendered: Boolean

        /** Reference to the current Request object */
        var request: Request

        /** Reference to the current View object 
UNUSED - MOB -- better to set in Request
        var view: View
         */

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
            Static factory method to create and initialize a controller. The controller class is specified by 
            params["controller"] which should be set to the controller name without the "Controller" suffix. 
            This call expects the controller class to be loaded. Called by Mvc.load().
            @param request Web request object
            @param cname Controller class name. This should be the name of the Controller class without the "Controller"
                suffix.
         */
        static function create(request: Request, cname: String = null): Controller {
            cname ||= (request.params.controller.toPascal() + "Controller")
            _initRequest = request
            let c: Controller = new global[cname](request)
            c.request = request
            _initRequest = null
            return c
        }

        /** 
            Create and initialize a controller. This may be called directly by class constructors or via 
            the Controller.create factory method.
            @param req Web request object
         */
        function Controller(req: Request) {
            /*  _initRequest may be set by create() to allow subclasses to omit constructors */
            controllerName = typeOf(this).trim("Controller") || "-DefaultController-"
            request = req || _initRequest
            if (request) {
                request.controller = this
                log = request.log
                params = request.params
                config = request.config
                if (config.database) {
                    openDatabase(request)
                }
            }
        }

        /** 
            Run a filter function after running the action
            @param fn Function callback to invoke
            @param options Filter options. 
            @option only Only run the filter for this action name
            @option except Run the filter for actions except this name
         */
        function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /** 
            Controller web application. This function will run the controller action method and return a response object. 
            The action method may be specified by the $aname parameter or it may be supplied via params.action.
            @param request Request object
            @param aname Optional action method name. If not supplied, params.action is consulted. If that is absent too, 
                "index" is used as the action method name.
            @return A response object hash {status, headers, body} or null if writing directly using the request object.
         */
        function app(request: Request, aname: String = null): Object {
            use namespace action
            actionName ||= aname || params.action || "index"
            params.action = actionName
            runFilters(_beforeFilters)
            let response
            if (!redirected && !rendered) {
                if (!this[actionName]) {
                    if (!viewExists(actionName)) {
                        response = this[actionName = "missing"]()
                    }
                } else {
                    response = this[actionName]()
                }
                if (!response && !rendered && !redirected && request.autoFinalizing) {
                    /* Run a default view */
                    renderView()
                }
                runFilters(_afterFilters)
            }
            request.autoFinalize()
            return response
        }

        /** 
            Run a filter function before running the action
            @param fn Function callback to invoke
            @param options Filter options. 
            @option only Only run the filter for this action name
            @option except Run the filter for actions except this name
         */
        function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /** 
            @duplicate Request.error
         */
        function error(msg: String): Void
            request.error(msg)

        /** 
            @duplicate Request.flash
         */
        function flash(key: String, msg: String): Void
            request.flash(key, msg)

        /** 
            @duplicate Request.header
         */
        function header(key: String): String
            request.header(key)

        /** 
            @duplicate Request.inform
         */
        function inform(msg: String): Void
            request.inform(msg)

        /** 
            @duplicate Request.makeUri
            @option controller The name of the controller to use in the URI.
            @option action The name of the action method to use in the URI.
         */
        function makeUri(location: Object): Uri
            request.makeUri(location)

        /** 
            Missing action method. This method will be called if the requested action routine does not exist.
         */
        action function missing(): Void {
            rendered = true
            throw "Missing Action: \"" + params.action + "\" could not be found for controller \"" + controllerName + "\""
        }

        /** 
            @duplicate Request.observe
         */
        function observe(name, observer: Function): Void
            request.observer(name, observer)

        /** 
            @duplicate Request.read
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
        function redirectAction(action: String): Void {
            if (request.route) {
                redirect({action: action})
            } else {
                redirect(request.uri.dirname.join(action))
            }
        }

        /** 
            Render the raw arguments back to the client. The args are converted to strings.
            @param args Arguments to write to the client
         */
        function render(...args): Void { 
            if (!rendered) {
                rendered = true
                request.write(...args)
            }
        }

        /**
            Render an error message as the response
            @param status Http status code to use
            @param msgs Error messages to send with the response
         */
        function renderError(status: Number, ...msgs): Void {
            if (!rendered) {
                rendered = true
                request.writeError(status, ...msgs)
            }
        }

        /** 
            Render a file's contents. 
            @param filename Path to the filename to send to the client
         */
        function renderFile(filename: Path): Void { 
            if (!rendered) {
                rendered = true
                request.sendFile(filename)
            }
        }

        /** 
            Render a partial response using template file. Does not set "rendered" to true.
            @param path Path to the template to render
            @param layouts Optional directory for layout files. Defaults to config.directories.layouts.
         */
        function renderPartialTemplate(path: Path, layouts: Path = null): Void { 
            request.filename = path
            layouts ||= config.directories.layouts
            let app = TemplateBuilder(request, { layouts: layouts } )
            log.debug(4, "renderPartial: \"" + path + "\"")
            Web.process(app, request)
        }

        /** 
            Render a view template
            @param viewName Name of the view to render. The view template filename will be constructed by joining
                the views directory with the controller name and view name. E.g. views/Controller/list.ejs
         */
        function renderView(viewName: String = null): Void {
            viewName ||= actionName
            renderTemplate(request.dir.join(config.directories.views, controllerName, viewName).
                joinExt(config.extensions.ejs))
        }

        /** 
            Render a view template from a path.
            @param path Path to the view template to render
            @param layouts Optional directory for layout files. Defaults to config.directories.layouts.
         */
        function renderTemplate(path: Path, layouts: Path = null): Void {
            if (!rendered) {
                rendered = true
                log.debug(4, "renderTemplate: \"" + path + "\"")
                request.filename = path
                layouts ||= config.directories.layouts
                let app = TemplateBuilder(request, { layouts: layouts } )
                Web.process(app, request)
            }
        }

        /** 
            Remove all defined filters on the Controller.
         */
        function removeFilters(): Void {
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
            @duplicate Request.warn
         */
        function warn(msg: String): Void
            request.warn(msg)

//  MOB -- these are not being used
        /** 
            Run a filter function wrapping the action
            @param fn Function callback to invoke
            @param options Filter options. 
            @option only Only run the filter for this action name
            @option except Run the filter for actions except this name
         */
        function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        /** 
            Low-level write data to the client. This will buffer the written data until either flush() or finalize() 
            is called.  This will not set the $rendered property.
            @duplicate Request.write
         */
        function write(...data): Number
            request.write(...data)

        /**************************************** Private ******************************************/
        /*
            Open database. Expects ejsrc configuration:

            mode: "debug",
            database: {
                class: "Database",
                adapter: "sqlite3",
                debug: { name: "db/blog.sdb", trace: true },
                test: { name: "db/blog.sdb", trace: true },
                production: { name: "db/blog.sdb", trace: true },
            }
         */
        private function openDatabase(request: Request) {
            let dbconfig = config.database
            let dbclass = dbconfig["class"]
            let profile = dbconfig[config.mode]
            if (dbclass && dbconfig.adapter && profile.name) {
                if (dbconfig.module && !global[dbclass]) {
                    global.load(dbconfig.module + ".mod")
                }
                new global[dbclass](dbconfig.adapter, request.dir.join(profile.name), profile.trace)
            }
        }

        /* 
            Run the before/after filters. These are typically used to handle authorization and similar tasks
         */
        private function runFilters(filters: Array): Void {
            for each (filter in filters) {
                let [fn, options] = filter
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
            let path = request.dir.join(config.directories.views, controllerName, name).joinExt(config.extensions.ejs)
            if (path.exists) {
                return true
            }
            return null
        }

        /********************************************  LEGACY 1.0.2 ****************************************/

        /** 
            Old appUrl routine
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get appUrl()
            request.home.toString().trimEnd("/")

        /** 
            Old makeUrl routine
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String
            makeUri({ path: action })

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function resetFilters(): Void
            removeFilters()
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
