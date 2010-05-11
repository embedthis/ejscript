/*
    Loader.es - CommonJS module loader with require() support.
 */

module ejs.cjs {

    /** 
        Loader for CommonJS modules
        @param id Module identifier. May be top level or may be an identifier relative to the loading script.
        @returns An object hash of exports from the module
        @spec ejs
        @stability prototype
     */
    function require(id: String): Object
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
        private static var config

        //  UNUSED - not yet used
        static function init(mainId: String? = null) {
            require.main = mainId
        }

        /**
            Load a CommonJS module. The module is loaded only once unless it is modified.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
         */
        public static function require(id: String): Object {
            let path: Path = locate(id)
            let exports = signatures[path]
            if (!exports || path.modified > timestamps[path]) {
                return load(id, path)
            }
            return exports
        }

        /** 
            Load a CommonJS module and return the exports object. After the first load, the CJS module will be compile
            and cached as a byte-code module.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
            @param path Optional path to the physical file corresponding to the module. If the module source code has
                changed, it will be re-compiled and then cached.
            @param codeReader Optional function to provide script code to use instead of reading from the path. 
         */
        public static function load(id: String, path: Path, codeReader: Function? = null): Object {
            let initializer, code
            if (path) {
                let cache: Path = cached(path)
                if (cache && cache.exists && cache.modified >= path.modified) {
                    App.log.debug(4, "Use cache for: " + path)
                    initializer = global.load(cache)
                } else {
                    if (codeReader) {
                        code = codeReader(path)
                    } else {
                        code = path.readString()
                        code = wrap(code)
                    }
                    App.log.debug(4, "Recompile module to: " + cache)
                    initializer = eval(code, cache)
                }
                timestamps[path] = path.modified
            } else {
//  MOB BUG - code doesn't exist
                code = wrap(code)
                initializer = eval(code, "mob.mod")
            }
            signatures[path] = exports = {}
            initializer(require, exports, {id: id, path: path}, null)
            return exports
        }

        /** @hide */
        public static function cached(path: Path, cachedir: Path? = null): Path {
            config ||= App.config
            if (path && config.cache.enable) {
                let dir = cachedir || Path(config.directories.cache) || Path("cache")
                if (dir.exists) {
                    return Path(dir).join(md5(path)).joinExt('.mod')
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
         */
        private static function locate(id: Path) {
            if (id.exists) {
                return id
            } 
            config ||= App.config
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
            throw new IOError("Can't find module \"" + id + "\"")
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
