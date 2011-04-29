/**
    Mvc.es -- Model View Controller (MVC) web app management
 */

module ejs.web {

    /** 
        The Mvc class manages the loading and initialization of MVC web applications
        @stability prototype
        @spec ejs
     */
    public class Mvc {

        static var apps = {}

        /*  
            Default configuration for MVC apps. This layers over App.defaultConfig and ejs.web::defaultConfig.
         */
        private static var defaultConfig = {
            //  MOB -- change to dirs
            directories: {
                bin: Path("bin"),
                db: Path("db"),
                controllers: Path("controllers"),
                models: Path("models"),
                src: Path("src"),
                static: Path("static"),
            },
            mvc: {
                //  MOB -- what is this?
                app: "",
                //  MOB - should be moved to files
                appmod: "App.mod",
                view: {
                    connectors: { },
                    formats: { },
                },
            },
        }

        private static var loaded: Object = {}
        private static const EJSRC = "ejsrc"

        private static function initMvc(): Void {
            blend(App.config, defaultConfig, false)
        }
        initMvc()

        /*
            Load the app/ejsrc and defaultConfig. Blend with the HttpServer.config
            @return The configuration object
         */
        private function loadConfig(request: Request): Object {
            let config = request.config
            let path = request.dir.join(EJSRC)
            if (request.dir != request.server.documentRoot && path.exists) {
                /* This is an app specific ejsrc */
                let appConfig = path.readJSON()
                /* Clone to get a request private copy of the configuration before blending "app/ejsrc" */
                config = request.config = request.config.clone()
                blend(config, appConfig, true)
                let dirs = config.directories
                for each (key in ["bin", "db", "controllers", "models", "src", "static"]) {
                    dirs[key] = request.dir.join(dirs[key])
                }
                App.updateLog()
            }
/* FUTURE
            //  Create per-MVC app logs
            if (config.log) {
                logger = new Logger("request", App.log, config.log.level)
            }
            if (config.mvc.app) {
                // Load custom MVC app script and use it 
                let script = request.dir.join(config.mvc.app)
                if (script.exists) {
                    startup = Loader.load(script, script, config).app
                }
            }
*/
            return config
        }


//  MOB -- rename to load?
        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
         */
        public function init(request: Request): Void {
            let config = loadConfig(request)
            let dirs = config.directories
            let appmod = dirs.cache.join(config.mvc.appmod)

            if (config.cache.flat) {
                if (!global.BaseController) {
                    global.load(appmod)
                }
            } else {
                let ext = config.extensions
                let dir = request.dir
                request.log.debug(4, "MVC init at \"" + dir + "\"")

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.cache.reload) {
                    deps = [dir.join(EJSRC)]
                    files = dirs.models.find("*" + ext.es)
                    files += dirs.src.find("*" + ext.es)
                    files += [dirs.controllers.join("Base").joinExt(ext.es)]
                }
                loadComponent(request, appmod, files, deps)

                /* Load controller */
                let params = request.params
                if (!params.controller) {
                    throw new StateError("No controller specified by route: " + request.route.name)
                }
                let controller = params.controller = params.controller.toPascal()
                let mod = dirs.cache.join(controller).joinExt(ext.mod)
                if (controller != "Base") {
                    if (!global[controller + "Controller"] && mod.exists && !config.cache.reload) {
                        loadComponent(request, mod)
                    } else {
                        files = [dirs.controllers.join(controller).joinExt(ext.es)]
                        deps = [dirs.controllers.join("Base").joinExt(ext.es)]
                        loadComponent(request, mod, files, deps)
                    }
                }
            }
            // FUTURE request.logger = logger
        }

        private function rebuildComponent(request: Request, mod: Path, files: Array) {
            let code = "require ejs.web\n"
            for each (file in files) {
                let path = Path(file)
                if (!path.exists) {
                    request.status = Http.NotFound
                    throw new StateError("Can't find required component: \"" + path + "\"")
                }
                code += path.readString()
            }
            request.log.debug(4, "Rebuild component: " + mod + " files: " + files)
            eval(code, mod)
        }

        /** 
            Load a component. This will load a module and optionally recompile if the given dependency paths are
            more recent than the module itself. If recompilation occurs, the result will be cached in the supplied module.
            @param request Request object
            @param mod Path to the module to load
            @param files Files to compile into the module
            @param deps Extra file dependencies
         */
        public function loadComponent(request: Request, mod: Path, files: Array? = null, deps: Array? = null) {
            let rebuild = false
            if (mod.exists && request.config.cache.reload) {
                let when = mod.modified
                for each (file in (files + deps)) {
                    if (file.exists && file.modified > when) {
                        rebuild = true
                    }
                }
            }
            if (rebuild) {
                rebuildComponent(request, mod, files)
            } else if (loaded[mod]) {
                request.log.debug(4, "Mvc.loadComponent: component already loaded: " + mod)
            } else {
                try {
                    request.log.debug(4, "Mvc.loadComponent: load component : " + mod)
                    global.load(mod)
                    loaded[mod] = new Date
                } catch (e) {
                    request.log.debug(4, "Mvc.loadComponent: Load failed, rebuild component: " + mod)
                    rebuildComponent(request, mod, files)
                }
            }
        }
    }

    /**
        MVC request handler.  
        @param request Request object
        @return A response hash (empty). MVC apps use Request methods directly to set status, headers and response body.
        @spec ejs
        @stability prototype
     */
    function MvcApp(request: Request): Object
        MvcBuilder(request)

//  MOB -- update doc
    /** 
        MVC builder for use in routing tables. The MVC builder function can be included directly in Route table entries.
        @param request Request object. 
        @return A web script function that services a web request.
        @example:
          { name: "index", builder: MvcBuilder, match: "/" }
        @spec ejs
        @stability prototype
     */
    function MvcBuilder(request: Request): Function {
        //  MOB OPT - Currently Mvc has no state so really don't need an Mvc instance
        let mvc: Mvc = Mvc.apps[request.dir] || (Mvc.apps[request.dir] = new Mvc(request))
        //  MOB -- rename to load?
        mvc.init(request)
        let cname: String = request.params.controller + "Controller"
        //  MOB - rename app to be more unique
        return Controller.create(request, cname).app
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
