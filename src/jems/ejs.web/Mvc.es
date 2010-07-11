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
                //  MOB - should be moved to files
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
        private static var loaded: Object = {}

        //  MOB -- where should this come from?
        private static const EJSRC = "ejsrc"

        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
            @returns The exports object
         */
        public static function load(request: Request): Object {
            request.dir = request.server.serverRoot
            let dir = request.dir
            //  MOB -- should be in filenames in config
            let path = dir.join(EJSRC)
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

            //  MOB -- good to have a single reload-app file 
            /* Load App */
            let appmod = dir.join(dirs.cache, mvc.appmod)
            let files, deps
            if (config.cache.reload) {
                deps = [dir.join(EJSRC)]
                files = dir.join(dirs.models).find("*" + ext.es)
                files += dir.join(dirs.src).find("*" + ext.es)
                files += [dir.join(dirs.controllers, "Base").joinExt(ext.es)]
            }
            loadComponent(request, appmod, files, deps)

            /* Load controller */
            let controller = request.params.controller
            let ucontroller = controller.toPascal()
            let mod = dir.join(dirs.cache, ucontroller).joinExt(ext.mod)
            if (!mod.exists || config.cache.reload) {
                files = [dir.join(dirs.controllers, ucontroller).joinExt(ext.es)]
                deps = [dir.join(dirs.controllers, "Base").joinExt(ext.es)]
                loadComponent(request, mod, files, deps)
            } else {
                loadComponent(request, mod)
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
        public static function loadComponent(request: Request, mod: Path, files: Array? = null, deps: Array? = null) {
            let rebuild
            if (mod.exists) {
                rebuild = false
                if (request.config.cache.reload) {
                    let when = mod.modified
                    for each (file in (files + deps)) {
                        if (file.exists && file.modified > when) {
                            rebuild = true
                        }
                    }
                }
            } else {
                rebuild = true
            }
            if (rebuild) {
                let code = "require ejs.web\n"
                for each (file in files) {
                    let path = Path(file)
                    if (!path.exists) {
                        throw "Can't find required component: \"" + path + "\""
                    }
                    code += path.readString()
                }
                request.log.debug(4, "Rebuild component: " + mod + " files: " + files)
                eval(code, mod)

            } else if (!loaded[mod]) {
                request.log.debug(4, "Reload component : " + mod)
                global.load(mod)
                loaded[mod] = new Date

            } else {
                request.log.debug(4, "Use existing component: " + mod)
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
