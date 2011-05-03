/*
    Controller.es -- MVC Controller class.
 */

module ejs.web {
    /**
        Namespace for all action methods 
     */
    namespace action = "action"

    //  DOC - need more doc here on controllers
    /** 
        Web framework controller class. The Controller class is part of the Ejscript Model View Controller (MVC) web
        framework. Controller class instances can accept web requests and direct them to action methods for servicing
        which generates the response. Controllers are responsible for either generating output for the client or invoking
        a View which will create the response. By convention, all Controllers should be defined with a "Controller" 
        suffix. This permits similar Controller and Model classes to exist in the same namespace.

        Action methods will autoFinalize by calling Request.autoFinalize unless Request.dontAutoFinalize has been called.
        If the Controller action wants to keep the request connection to the client open, you must call dontAutoFinalize
        before returning from the action.
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

        private var _afterCheckers: Array
        private var _beforeCheckers: Array

        /** Name of the Controller action method being run for this request */
        var actionName:  String 

        /** Configuration settings. This is a reference to $ejs.web::Request.config */
        var config: Object 

//  TODO -- rename to "name"
        /** Pascal case controller name */
        var controllerName: String

        /** Logger stream - reference to $ejs.web::Request.log */
        var log: Logger

        /** Form and query parameters. This is a reference to the $ejs.web::Request.params object. */
        var params: Object

        /** Reference to the current $ejs.web::Request object */
        var request: Request

        /***************************************** Convenience Getters  ***************************************/

        /** @duplicate ejs.web::Request.absHome */
        function get absHome(): Uri 
            request ? request.absHome : null

        /** @duplicate ejs.web::Request.home */
        function get home(): Uri 
            request ? request.home : null

        /** @duplicate ejs.web::Request.pathInfo */
        function get pathInfo(): String 
            request ? request.pathInfo : null

        /** @duplicate ejs.web::Request.session */
        function get session(): Session 
            request ? request.session : null

        /** @duplicate ejs.web::Request.uri */
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
printHash("CREATE CONTROLLER INSTANCE request ", request)
            request.params.controller = request.params.controller.toPascal()
            cname ||= (request.params.controller + "Controller")
            _initRequest = request
            let c: Controller = new global[cname](request)
            c.request = request
            _initRequest = null
            return c
        }

        /** 
            Create and initialize a controller. This may be called directly or via 
            the Controller.create factory method.
            @param req Web request object
         */
        function Controller(req: Request = null) {
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
                if (request.method == "POST") {
                    before(checkSecurityToken)
                }
            }
        }

        /** 
            Run an check function after running the action
            @param fn Function callback to invoke
            @param options Check function options. 
            @option only [String|Array] Only run the checker for this action name. This can be a string action name or
                an array of action names.
            @option except [String|Array] Run the check function for all actions except this name.
                This can be a string action name or an array of action names.
         */
        function after(fn, options: Object = null): Void {
            _afterCheckers ||= []
            _afterCheckers.append([fn, options])
        }

        //  MOB - rename
        /** 
            Controller web application. This function will run a controller action method and return a response object. 
            The action method may be specified by the $aname parameter or it may be supplied via 
            $ejs.web::Request.params.action.
            @param request Request object
            @param aname Optional action method name. If not supplied, params.action is consulted. If that is absent too, 
                "index" is used as the action method name.
            @return A response object hash {status, headers, body} or null if writing directly using the request object.
         */
        function app(request: Request, aname: String = null): Object {
print("11 IN CONTROLLER APP")
            let ns = params.namespace || "action"
            actionName ||= aname || params.action || "index"
            params.action = actionName
print("AA")
            runCheckers(_beforeCheckers)
            let response
print("BB")
            if (!request.finalized && request.autoFinalizing) {
print("CC")
                if (!(ns)::[actionName]) {
                    if (!viewExists(actionName)) {
                        response = "action"::missing()
                    }
                } else {
                    App.log.debug(3, "Run action " + actionName)
print("C1 " + actionName)
                    response = (ns)::[actionName]()
print("C2")
                }
print("C3")
                if (response && !response.body) {
                    throw "Response object is missing a \"body\""
                }
print("C4")
breakpoint()
                if (!response && !request.responded && request.autoFinalizing) {
                    /* Run a default view */
print("C5")
                    writeView()
                }
            }
print("DD")
            runCheckers(_afterCheckers)
            if (!response) {
                request.autoFinalize()
            }
print("LEAVING CONTROLLER APP")
            return response
        }

        /** @duplicate ejs.web::Request.autoFinalize */
        function autoFinalize(): Void
            request.autoFinalize()

        /** @duplicate ejs.web::Request.autoFinalizing */
        function get autoFinalizing(): Boolean
            request.autoFinalizing

        /** 
            Run an action checker before running the action. If the checker function writes a response, the normal
            processing of the requested action will be prevented. Note that checkers do not autoFinalize so if the
            checker does write a response, it must call finalize.
            @param fn Function callback to invoke
            @param options Checker options. 
            @option only [String|Array] Only run the checker for this action name. This can be a string action name or
                an array of action names.
            @option except [String|Array] Run the checker for all actions except this name
                This can be a string action name or an array of action names.
         */
        function before(fn, options: Object = null): Void {
            _beforeCheckers ||= []
            _beforeCheckers.append([fn, options])
        }

        /** @duplicate ejs.web::Request.dontAutoFinalize */
        function dontAutoFinalize(): Void
            request.dontAutoFinalize()

        /** 
            @duplicate ejs.web::Request.error
         */
        function error(msg: String): Void
            request.error(msg)

        /** @duplicate ejs.web::Request.finalize */
        function finalize(): Void
            request.finalize()

        /** @duplicate ejs.web::Request.finalized */
        function get finalized(): Boolean
            request.finalized

        /** 
            @duplicate ejs.web::Request.flash
         */
        function get flash(): Object
            request.flash

        /** @duplicate ejs.web::Request.flush */
        function flush(dir: Number = Stream.WRITE): Void
            request.flush(dir)

        /** 
            @duplicate ejs.web::Request.header
         */
        function header(key: String): String
            request.header(key)

        /** @duplicate ejs.web::Request.inform */
        function inform(msg: String): Void
            request.inform(msg)

        /** @duplicate ejs.web::Request.link */
        function link(target: Object): Uri
            request.link(target)

        /** 
            Missing action method. This method will be called if the requested action routine does not exist.
            It should be overridden in user controller classes by using the "override" keyword.
         */
        action function missing() {
            throw "Missing Action: " + params.action + ": could not be found for controller: " + controllerName
            return ""
        }

        /** @duplicate ejs.web::Request.notify */
        function notify(key: String, msg: String): Void
            request.notify(key, msg)

        /** @duplicate ejs.web::Request.on */
        function on(name, observer: Function): Void
            request.on(name, observer)

        /** @duplicate ejs.web::Request.read */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 
            request.read(buffer, offset, count)

        /** 
            Redirect the client to the given URL
            @param where Url to redirect the client toward. This can be a relative or absolute string URL or it can be
                a hash of URL components. For example, the following are valid inputs: "../index.ejs", 
                "http://www.example.com/home.html", {action: "list"}.
            @param status Http status code to use in the redirection response. Defaults to 302.
         */
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void
            request.redirect(where, status)

        /** 
            Remove all defined checkers on the Controller.
         */
        function removeCheckers(): Void {
            _beforeCheckers = null
            _afterCheckers = null
        }

        /** @duplicate ejs.web::Request.setHeader */
        function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /** @duplicate ejs.web::Request.setHeaders */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void
            request.setHeaders(headers, overwrite)

        /** @duplicate ejs.web::Request.setStatus */
        function setStatus(status: Number): Void
            request.status = status

        /** 
            Low-level write data to the client. This will buffer the written data until either flush() or finalize() 
            is called.
            If an action method does call a write data back to the client and has not called finalize() or 
            dontAutoFinalize(), a default view template will be generated when the action method returns. 
            @param args Arguments to write to the client. The args are converted to strings.
            @return The number of bytes written to the client
         */
        function write(...args): Number
            request.write(...args)

        /** 
            @duplicate ejs.web::Request.warn
         */
        function warn(msg: String): Void
            request.warn(msg)

        /**
            @duplicate ejs.web::Request.writeContent
         */
        function writeContent(data): Void
            request.writeContent(data)

        /**
            Render an error message as the response.
            This call sets the response status and writes a HTML error message with the given arguments back to the client.
            @param status Http response status code
            @param msgs Error messages to send with the response
         */
        function writeError(status: Number, ...msgs): Void
            request.writeError(status, ...msgs)

        /** 
            Render file content back to the client.
            This call writes the given file contents back to the client.
            @param filename Path to the filename to send to the client
         */
        function writeFile(filename: Path): Void
            request.writeFile(filename)

        /** 
            Render a partial response using template file.
            @param path Path to the template to render to the client
            @param options Additional options.
            @option layout Optional layout template. Defaults to config.directories.layouts/default.ejs.
         */
        function writePartialTemplate(path: Path, options: Object = {}): Void { 
            request.filename = path
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = Path(config.directories.layouts).join(config.web.view.layout)
            }
            let app = TemplateBuilder(request, options)
            log.debug(4, "writePartialTemplate: \"" + path + "\"")
            Web.process(app, request, false)
        }

        /** 
            Render a view template.
            This call writes the result of running the view template file back to the client.
            @param viewName Name of the view to render to the client. The view template filename will be constructed by 
                joining the views directory with the controller name and view name. E.g. views/Controller/list.ejs.
            @param options Additional options.
            @option controller Optional controller for the view.
            @option layout Optional layout template. Defaults to config.directories.layouts/default.ejs.
         */
        function writeView(viewName = null, options: Object = {}): Void {
            let controller = options.controller || controllerName
            viewName ||= options.action || actionName
            if (options.layout === undefined) {
                options.layout = config.directories.layouts.join(config.web.view.layout)
            }
            writeTemplate(config.directories.views.join(controller, viewName).joinExt(config.extensions.ejs), options)
        }

        /** 
            Render a view template from a path.
            This call writes the result of running the view template file back to the client.
            @param path Path to the view template to render and write to the client.
            @param options Additional options.
            @option layout Optional layout template. Defaults to config.directories.layouts/default.ejs.
         */
        function writeTemplate(path: Path, options: Object = {}): Void {
print("\n\nWRITE TEMPLATE " + path)
            log.debug(4, "writeTemplate: \"" + path + "\"")
            let saveFilename = request.filename
            request.filename = path
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = config.directories.layouts.join(config.web.view.layout)
            }
            let app = TemplateBuilder(request, options)
print(">>>>>>>>>> app" + app)
            Web.process(app, request, false)
print("<<<<<<<<<<")
            request.filename = saveFilename
        }

        /** 
            Render a view template from a string literal.
            This call writes the result of running the view template file back to the client.
            @param page String literal containing the view template to render and write to the client.
            @param options Additional options.
            @option layout Path layout template. Defaults to config.directories.layouts/default.ejs.
         */
        function writeTemplateLiteral(page: String, options: Object = {}): Void {
            log.debug(4, "writeTemplateLiteral")
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = config.directories.layouts.join(config.web.view.layout)
            }
            options.literal = page
            let app = TemplateBuilder(request, options)
            Web.process(app, request, false)
        }

        /**************************************** Private ******************************************/

        private function checkSecurityToken()
            request.checkSecurityToken()

        /*
            Open database. Expects ejsrc configuration:

            mode: "debug",
            database: {
                module: "name.mod",
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
            let options = dbconfig[config.mode]
            if (dbclass) {
                if (dbconfig.module && !global[dbclass]) {
                    global.load(dbconfig.module + ".mod")
                }
                let module = dbconfig.module || "public"
                options.dir = request.dir
                new (module)::[dbclass](dbconfig.adapter, options)
            }
        }

        /* 
            Run the before/after checkers. These are typically used to handle authorization and similar tasks
         */
        private function runCheckers(checkers: Array): Void {
            for each (checker in checkers) {
                let [fn, options] = checker
                if (options) {
                    if (only = options.only) {
                        if ((only is String && actionName != only) || (only is Array && !only.contains(actionName))) {
                            continue
                        }
                    } 
                    if (except = options.except) {
                        if ((except is String && actionName == except) || (except is Array && except.contains(actionName))) {
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
            let path = config.directories.views.join(controllerName, name).joinExt(config.extensions.ejs)
            if (path.exists) {
                return true
            }
            return null
        }

        /********************************************  LEGACY 1.0.2 ****************************************/

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get absUrl()
            absHome

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function afterFilter(fn, options: Object = null): Void
            after(fn, options)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get appUrl()
            home.trimEnd("/")

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function beforeFilter(fn, options: Object = null): Void
            before(fn, options)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function createSession(timeout: Number): Void
            request.createSession(timeout)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function destroySession(): Void
            request.destroySession()

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function discardOutput(): Void {
            //  No supported
            true
        }
            
        /**
            escapeHtml, html is now a global in Utils.es
         */

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get host()
            request.server

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function keepAlive(on: Boolean): Void {
            // Not supported 
            true
        }

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function loadView(path): Void
            writeTemplate(path)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String
            link({ action, id, query })

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function redirectUrl(uri: String, status: Number = 302): Void
            redirect(uri, status)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function render(...args): Void
            write(...args)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderFile(filename: String): Void
            writeFile(filename)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderRaw(...args): Void
            write(...args)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderView(name: String = null): Void
            writeView(name)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function reportError(status: Number, msg: String, e: Object = null): Void
            writeError(status, msg + e)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function resetFilters(): Void
            removeCheckers()

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function sendError(status, ...msgs): Void
            writeError(status, ...msgs)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function setCookie(name: String, value: String, path: String = null, domain: String = null,
                lifetime: Number = 0, secure: Boolean = false): Void  {
            request.setCookie(name, 
                { value: value, path: path, domain: domain, lifetime: Date().future(lifetime * 1000), secure: secure})
        }

    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
