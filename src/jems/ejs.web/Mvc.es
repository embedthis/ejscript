/**
    Mvc.es -- MVC web app management
 */

module ejs.web {

    require ejs.cjs

    /** 
        The Mvc manages the loading of MVC applications
        @stability prototype
        @spec ejs
     */
    public class Mvc {
        /*  
            Default configuration for MVC apps. This layers over App.defaultConfig and ejs.web::defaultConfig.
         */
        private static var defaultConfig = {
            cache: {
                enable: true,
                reload: true,
            },
            directories: {
                bin: Path("bin"),
                db: Path("db"),
                cache: Path("cache"),
                controllers: Path("controllers"),
                layouts: Path("views/layouts"),
                models: Path("models"),
                views: Path("views"),
                src: Path("src"),
                web: Path("web"),
            },
            extensions: {
                es: "es",
                ejs: "ejs",
                mod: "mod",
            },
            mvc: {
                //  MOB -- what is this?
                app: "",
                appmod: "App.mod",
                views: {
                    connectors: { },
                    formats: { },
                },
            },
        }

        /* References into the config state */
        private static var mvc: Object
        private static var dirs: Object
        private static var ext: Object

        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
            @returns The exports object
         */
        public static function load(request: Request): Object {
            request.dir = request.server.serverRoot
            let dir = request.dir
            let path = dir.join("ejsrc")
            let config = request.config
            if (path.exists) {
                let appConfig = deserialize(path.readString())
                /* Clone to get a request private copy of the configuration */
                /* MOB - why do this? */
                config = request.config = request.config.clone()
                blend(config, appConfig, true)
/* MOB - future create a new logger
                if (app.config.log) {
                    request.logger = new Logger("request", App.log, log.level)
                    if (log.match) {
                        App.log.match = log.match
                    }
                }
 */
            }
            blend(config, defaultConfig, false)
            mvc = config.mvc
            dirs = config.directories
            ext = config.extensions
            //  MOB temp
            App.log.level = config.log.level

            let exports
            if (mvc.app) {
    //  MOB -- what is this?
                let app = dir.join(mvc.app)
                if (app.exists) {
                    exports = Loader.load(app, app)
                }
            }
            return exports || { 
                app: function (request: Request): Object {
                    //  BUG - can't use Mvc.init as "this" has been modified from Mvc to request
                    global["Mvc"].init(request)
                    let controller = Controller.create(request)
                    return controller.run(request)
                }
            }
        }

        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
         */
        public static function init(request: Request): Void {
            let config = request.config
            let dir = request.dir

            /* Load App */
            let mod = dir.join(dirs.cache, mvc.appmod)
            let deps
            if (config.mode == "debug") {
                deps = []
                deps += dir.join(dirs.models).find("*" + ext.es)
                deps += dir.join(dirs.src).find("*" + ext.es)
                deps += [dir.join(dirs.controllers, "Base").joinExt(ext.es)]
            }
            loadComponent(request, mod, deps)

            /* Load controller */
            let controller = request.params.controller
            let ucontroller = controller.toPascal()
            mod = dir.join(dirs.cache, ucontroller).joinExt(ext.mod)
            if (!mod.exists || config.cache.reload) {
                loadComponent(request, mod, [dir.join(dirs.controllers, ucontroller).joinExt(ext.es)])
            } else {
                loadComponent(request, mod)
            }
        }

        /** 
            Load a component. This will load a module and optionally recompile if the given dependency paths are
            more recent than the module itself. If recompilation occurs, the result will be cached in the supplied module.
            @param request Request object
            @param mod Path to the module to load
            @param deps Module dependencies
         */
        public static function loadComponent(request: Request, mod: Path, deps: Array? = null) {
            let rebuild
            if (mod.exists) {
                rebuild = false
                let when = mod.modified
                for each (dep in deps) {
                    if (dep.exists && dep.modified > when) {
                        rebuild = true
                    }
                }
            } else {
                rebuild = true
            }
            if (rebuild) {
                let code = ""
                request.log.debug(4, "ejs.mvc: Check dependencies:", deps)
                for each (dep in deps) {
                    let path = Path(dep)
                    if (!path.exists) {
                        throw "Can't find required component: \"" + path + "\""
                    }
                    code += path.readString()
                }
                request.log.debug(4, "Rebuild component " + mod)
                eval(code, mod)
            } else {
                request.log.debug(4, "Load component from cache " + mod)
                global.load(mod)
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
