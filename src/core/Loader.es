/**
    Loader.es - CommonJS module loader with require() support.
 */

module ejs {

    /** 
        Loader for CommonJS modules
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
        private static var signatures = {}
        private static var timestamps = {}
        private static const defaultExtensions = [".es", ".js"]

        //  UNUSED - not yet used
        static function init(mainId: String? = null) {
            require.main = mainId
        }

        /**
            Load a CommonJS module. The module is loaded only once unless it is modified.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
            @param return a hash of exported properties
         */
        public static function require(id: String, config: Object = App.config): Object {
            let exports = signatures[id]
            if (!exports || config.cache.reload) {
                let path: Path = locate(id, config)
                if (path.modified > timestamps[path]) {
                    return load(id, path, config)
                }
            }
            return exports
        }

        /** 
            Load a CommonJS module and return the exports object. After the first load, the CJS module will be compile
            and cached as a byte-code module.
            @param id Unique name of the module to load. The id may be a unique ID, an absolute path, relative path or a 
                path fragment that is resolved relative to the App search path. Ids may or may not include a ".es" or 
                ".js" extension.
            @param path Optional path to the physical file corresponding to the module. If the module source code has
                changed, it will be re-compiled and then cached.
            @param codeReader Optional function to provide script code to use instead of reading from the path. 
            @param return a hash of exported properties
         */
        public static function load(id: String, path: Path, config = App.config, codeReader: Function = null): Object {
            let initializer, code
            let cache: Path = cached(id, config)
            if (path) {
                if (cache && cache.exists && cache.modified >= path.modified) {
                    App.log.debug(4, "Use cache for: " + path)
                    initializer = global.load(cache)
                } else {
                    if (codeReader) {
                        code = codeReader(id, path)
                    } else {
                        if (!path.exists) {
                            throw "Cannot find \"" + path + "\"."
                        }
                        code = wrap(path.readString())
                    }
                    if (cache) {
                        App.log.debug(4, "Recompile module to: " + cache)
                    }
                    initializer = eval(code, cache)
                }
                timestamps[path] = path.modified
            } else {
                if (codeReader) {
                    code = codeReader(id, path)
                } else {
                    throw "Must provide a codeReader if path is not specified"
                }
                initializer = eval(code, cache)
            }
            signatures[path] = exports = {}
            //  MOB -- implement system?
            //  function initializer(require, exports, module, system)
            initializer(require, exports, {id: id, path: path}, null)
            return exports
        }

        /** @hide */
        public static function cached(id: Path, config = App.config, cachedir: Path = null): Path {
            config ||= App.config
            if (id && config.cache.enable) {
                let dir = cachedir || Path(config.directories.cache) || Path("cache")
                if (dir.exists) {
                    return Path(dir).join(md5(id)).joinExt('.mod')
                } else {
                    App.log.error("Can't find cache directory: " + dir)
                }
            }
            return null
        }

        /** @hide */
        public static function wrap(code: String): String
            "(function(require, exports, module, system) {\n" + code + "\n})"

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
        public static function setConfig(newConfig): Void
            config = newConfig
    }
}
