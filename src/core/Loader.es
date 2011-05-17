/**
    Loader.es - CommonJS module loader with require() support.
 */

module ejs {

    /** 
        Loader for CommonJS modules. The Loader class provides provides a $require() function to load
            script files.
        @param id Module identifier. May be top level or may be an identifier relative to the loading script.
        @returns An object hash of exports from the module
        @spec ejs
        @stability prototype
     */
    public function require(id: String): Object
        Loader.require(id)

    /** 
        CommonJS loader class. This is public do assist dynamic loading of ejs.cjs so namespace qualification is not needed.
        @spec ejs
        @stability prototype
     */ 
    public class Loader {
        //  TODO - doc
        public  static var mainId
        public static var signatures = {}
        public static var initializers = {}
        private static var timestamps = {}
        private static const defaultExtensions = [".es", ".js"]

        /**
            UNUSED - not yet used
            @hide
         */
        public static function init(mainId: String? = null)
            require.main = mainId

        /**
            Register a CommonJS module initializer
            @param id Unique path ID for the module. This is typically a relative path to the module
            @param fn Module initialization function
            @return The function 
         */
        public static function register(id, fn): Function {
            Loader.initializers[id] = fn
            return fn
        }

        /**
            Load a CommonJS module. The module is loaded only once unless it is modified.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
            @return a hash of exported properties
         */
        public static function require(id: String, config: Object = App.config): Object {
            let exports = signatures[id]
            if (!exports || config.app.reload) {
                let path: Path = locate(id, config)
                if (path.modified > timestamps[path]) {
                    if (!global."ejs.cjs"::CommonSystem) {
                        /* On-demand loading of CommonJS modules */
                        global.load("ejs.cjs.mod")
                    }
                    return load(id, path, config)
                }
            }
            return exports
        }

        /** 
            Load a CommonJS module and return the exports object. After the first load, the script module will be compiled
            and cached as a byte-code module. It will be recompiled if the script source is modified or missing.
            @param id Unique name of the module to load. The id may be a unique ID, an absolute path, relative path or a 
                path fragment that is resolved relative to the App search path. Ids may or may not include a ".es" or 
                ".js" extension.
            @param path Optional path to the physical file corresponding to the module. If the module source code has
                changed, it will be re-compiled and then cached.
            @param codeReader Optional function to provide script code to use instead of reading from the path. 
            @return a hash of exported properties
         */
        public static function load(id: String, path: Path, config = App.config, codeReader: Function = null): Object {
            let initializer, code
            let cache: Path = cached(id, config)
            if (path) {
                if (cache && cache.exists && (!config.app.reload || cache.modified > path.modified)) {
                    /* Cache mod file exists and is current */
                    if (initializers[path]) {
                        App.log.debug(4, "Use memory cache for \"" + path + "\"")
                        initializer = initializers[path]
                        signatures[path] = exports = {}
                        initializer(require, exports, {id: id, path: path}, null)
                        return exports
                    }
                    App.log.debug(4, "Use disk cache for \"" + path + "\" from \"" + cache + "\"")
                    try { initializer = global.load(cache); } catch {}

                } else {
                    /* Missing or out of date cache mod file */
                    //  MOB - can we eliminate preloaded and just use reload?
                    if (initializers[path] && config.app.preloaded) {
                        //  Everything compiled flat - everything in App.mod
                        //  MOB -- warning. This prevents reload working. Should rebuild all and reload.
                        initializer = initializers[path]
                        signatures[path] = exports = {}
                        App.log.debug(4, "Use preloaded \"" + path + "\"")
                        initializer(require, exports, {id: id, path: path}, null)
                        return exports
                    }
                    if (codeReader) {
                        code = codeReader(id, path)
                    } else {
                        if (!path.exists) {
                            throw "Cannot find \"" + path + "\"."
                        }
                        code = wrap(id, path.readString())
                    }
                    if (cache) {
                        App.log.debug(4, "Recompile \"" + path + "\" to \"" + cache + "\"")
                    } else {
                        App.log.debug(4, "Compile \"" + path + "\" no caching")
                    }
                    initializer = eval(code, cache)
                }
                timestamps[path] = path.modified
            }
            if (initializer == null) {
                App.log.debug(4, "Compile \"" + path + "\" to \"" + cache + "\"")
                if (codeReader) {
                    code = codeReader(id, path)
                } else {
                    throw new StateError("Cannot load " + id + ". Must provide a codeReader if path is not specified.")
                }
                initializer = eval(code, cache)
            }
            //  TODO -- implement system and module?
            //  function initializer(require, exports, module, system)
            signatures[path] = exports = {}
            initializer(require, exports, {id: id, path: path}, null)
    /*
            if (exports.app.bound == this) {
                exports.app.bind(null)
            }
    */
            return exports
        }

        /** @hide */
        public static function cached(id: Path, config = App.config, cachedir: Path = null): Path {
            config ||= App.config
            if (id && config.app.cache) {
                let dir = cachedir || Path(config.dirs.cache) || Path("cache")
                if (dir.exists) {
                    return Path(dir).join(md5(id)).joinExt('.mod')
                } else {
                    App.log.error("Can't find cache directory: \"" + dir + "\"")
                }
            }
            return null
        }

        /** @hide */
        public static function wrap(id: String, code: String): String
            "Loader.register(\"" + id + "\", function(require, exports, module, system) {\n" + code + "\n})"

        /*  
            Locate a CommonJS module. The id can be an absolute path, a path with/without a "es" or "js" extension. 
            It will also search for the id relative to the App search path.
            @param id Path fragment to the module
            @return A full path to the module
         */
        private static function locate(id: Path, config = App.config) {
            if (id.exists) {
                return id
            } 
            //  TODO - need logging here
            let extensions = config.extensions || defaultExtensions
            for each (let dir: Path in App.search) {
                for each (ext in extensions) {
                    //  TODO - remove when typed expressions are enabled
                    let path: Path = Path(dir).join(id)
                    path = path.joinExt(ext)
                    if (path.exists) {
                        return path
                    }
                }
            }
            throw "Can't find \"" + id + "\""
        }

        /** 
            Set the configuration options hash for require to use. Loader uses the config.extensions field to
            determine the eligible file extensions to use when searching for modules.
            @param newConfig Configuration options hash.
         */
        public static function setConfig(newConfig: Object): Void
            config = newConfig
    }
}
