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
                views: Path("views"),
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

        /* App.config.dirs should be not be prefixed with any application paths */
        blend(App.config, defaultConfig, {overwrite: false})

        /** 
            Load an MVC application and the optional application specific ejsrc file
            @param home Base directory for the MVC app
            @param config Configuration object
         */
        function Mvc(home: Path, config = App.config) {
            this.config = config
            let path = home.join(EJSRC)
            if (path.exists) {
                let econfig = path.readJSON()
                let dirs = econfig.dirs
                for (let [key,value] in econfig.dirs) {
                    dirs[key] = home.join(value)
                }
                config = blend(config.clone(), econfig)
                App.updateLog()
            }
            if (config.database) {
                openDatabase(home)
            }
        }

        /**
            Factory to load an MVC application.
            @param request Request object
            @param dir Base directory containing the MVC application. Defaults to "."
            @param config Default configuration for the application
            @return An Mvc application object
          */
        public static function load(request: Request, dir: Path = ".", config = App.config): Mvc {
            if ((mvc = Mvc.apps[dir]) == null) {
                App.log.debug(2, "Load MVC application from \"" + dir + "\"")
                mvc = Mvc.apps[dir] = new Mvc(dir, config)
                let appmod = config.dirs.cache.join(config.mvc.appmod)

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.cache.app.reload) {
                    let dirs = config.dirs
                    let ext = config.extensions
                    deps = [dir.join(EJSRC)]
                    files = dirs.models.find("*" + ext.es)
                    files += dirs.src.find("*" + ext.es)
                    files += [dirs.controllers.join("Base").joinExt(ext.es)]
                }
                request ||= new Request("/")
                request.config = config
                mvc.loadComponent(request, appmod, files, deps)
                loaded[appmod] = new Date
            }
            return mvc
        }

        /*
            Load the required parts of the Mvc app
         */
        function loadRequest(request: Request) {
            request.config = config
            let dirs = config.dirs
            let appmod = dirs.cache.join(config.mvc.appmod)
            if (config.web.flat) {
                //  MOB - implement flat
                if (!global.BaseController) {
                    global.load(appmod)
                }
            } else {
                let ext = config.extensions
                let dir = request.dir
                request.log.debug(4, "MVC init at \"" + dir + "\"")

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.cache.app.reload) {
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
                    if (!global[controller + "Controller"] && mod.exists && !config.cache.app.reload) {
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
            if (mod.exists && request.config.cache.app.reload) {
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

        /**
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
            @hide
         */
        public static function openDatabase(home: Path) {
            let dbconfig = App.config.database
            if (dbconfig) {
                for each (kind in ["debug", "test", "production"]) {
                    if (dbconfig[kind]) {
                        dbconfig[kind].name = home.join(dbconfig[kind].name)
                    }
                }
                global.load("ejs.db.mod", {reload: false})
                blend(dbconfig, dbconfig[App.config.mode])
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

    //  MOB - who uses this?
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
        let mvc: Mvc = Mvc.load(request, request.dir, request.config)
        mvc.loadRequest(request)
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
