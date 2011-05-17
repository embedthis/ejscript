/**
    Mvc.es -- Model View Controller (MVC) web app management
 */

module ejs.web {

    /** 
        The Mvc class manages the loading and initialization of MVC web applications
        @stability prototype
        @spec ejs
     */
    class Mvc {

        static var apps = {}

        /*  
            Default configuration for MVC apps. This layers over App.defaultConfig and ejs.web::defaultConfig.
         */
        private static var defaultConfig = {
            dirs: {
                bin: Path("bin"),
                db: Path("db"),
                controllers: Path("controllers"),
                models: Path("models"),
                src: Path("src"),
                static: Path("static"),
            },
            mvc: {
                //  MOB - should be moved to files
                appmod: "App.mod",
                view: {
                    connectors: { },
                    formats: { },
                },
            },
        }

        private var config: Object
        private static var loaded: Object = {}
        private static const EJSRC = "ejsrc"

        blend(App.config, defaultConfig, false)

        /** 
            Load an MVC application and the optional application specific ejsrc file
            @param request Request object
         */
        function Mvc(request: Request) {
            config = request.config
            let path = request.dir.join(EJSRC)
            if (request.dir != request.server.documents && path.exists) {
                loadConfig(request, path)
                request.config = config
            }
            if (config.database) {
                openDatabase(request)
            }
        }

        /*
            Load the app/ejsrc and defaultConfig
         */
        private function loadConfig(request: Request, path: Path): Void {
            let appConfig = path.readJSON()
            config = blend(config.clone(), appConfig, true)
            let dirs = config.dirs
            for each (key in ["bin", "db", "controllers", "models", "src", "static"]) {
                dirs[key] = request.dir.join(dirs[key])
            }
            for (let [key, value] in dirs) {
                dirs[key] = Path(value)
            }
            App.updateLog()
        }

        /*
            Load the required parts of the Mvc app
         */
        function load(request: Request) {
            let dirs = config.dirs
            let appmod = dirs.cache.join(config.mvc.appmod)
            if (config.web.cache.flat) {
                if (!global.BaseController) {
                    global.load(appmod)
                }
            } else {
                let ext = config.extensions
                let dir = request.dir
                request.log.debug(4, "MVC init at \"" + dir + "\"")

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.app.reload) {
                    deps = [dir.join(EJSRC)]
                    files = dirs.models.find("*" + ext.es)
                    files += dirs.src.find("*" + ext.es)
                    files += [dirs.controllers.join("Base").joinExt(ext.es)]
                }
                loadComponent(request, appmod, files, deps)

                /* Load controller */
                let params = request.params
                if (!params.controller) {
                    throw "No controller specified by route: " + request.route.name
                }
                let controller = params.controller = params.controller.toPascal()
                let mod = dirs.cache.join(controller).joinExt(ext.mod)
                if (controller != "Base") {
                    if (!global[controller + "Controller"] && mod.exists && !config.app.reload) {
                        loadComponent(request, mod)
                    } else {
                        files = [dirs.controllers.join(controller).joinExt(ext.es)]
                        deps = [dirs.controllers.join("Base").joinExt(ext.es)]
                        loadComponent(request, mod, files, deps)
                    }
                }
            }
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
            if (mod.exists && request.config.app.reload) {
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

        /*
            Open database. Expects ejsrc configuration:
                mode: "debug",
                database: {
                    adapter: "sqlite",
                    module: "ejs.db.sqlite",
                    class: "Sqlite",
                    debug: { name: "db/blog.sdb", trace: true },
                    test: { name: "db/blog.sdb", trace: true },
                    production: { name: "db/blog.sdb", trace: true },
                }
         */
        private function openDatabase(request: Request) {
            let dbconfig = config.database
            if (dbconfig) {
                global.load("ejs.db.mod")
                blend(dbconfig, dbconfig[config.mode])
                new "ejs.db"::["Database"](dbconfig.adapter, dbconfig)
            }
        }

        private function rebuildComponent(request: Request, mod: Path, files: Array) {
            let code = "require ejs.web\n"
            for each (file in files) {
                let path = Path(file)
                if (!path.exists) {
                    request.status = Http.NotFound
                    throw "Can't find required component: \"" + path + "\""
                }
                code += path.readString()
            }
            request.log.debug(4, "Rebuild component: " + mod + " files: " + files)
            eval(code, mod)
        }

    }

    /**
        MVC request handler.  
        @param request Request object
        @return A response hash (empty). MVC apps use Request methods directly to set status, headers and response body.
        @spec ejs
        @stability prototype
     */
    function MvcApp(request: Request): Object {
        let app = MvcBuilder(request)
        return app(request)
    }

//  MOB -- update doc. Rename from builder?
    /** 
        MVC builder for use in routing tables. The MVC builder function can be included directly in Route table entries.
        @param request Request object. 
        @return A web script function that services a web request.
        @spec ejs
        @stability prototype
     */
    function MvcBuilder(request: Request): Function {
        let mvc: Mvc
        if ((mvc = Mvc.apps[request.dir]) == null) {
            App.log.debug(2, "Load MVC application from \"" + request.dir + "\"")
            mvc = Mvc.apps[request.dir] = new Mvc(request)
        }
        mvc.load(request)
        //  MOB - rename app to be more unique
        return Controller.create(request, request.params.controller + "Controller").app
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
