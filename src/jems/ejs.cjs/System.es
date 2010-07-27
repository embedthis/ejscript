/*
    System class. System properties for CommonJS.
 */

module ejs.cjs {

    enumerable dynamic class System {
        use default namespace public

        static var stdin: File
        static var stdout: File
        static var stderr: File
        static var args: Array
        static var env: Array
        static var fs: Object
        static var platform: String

        function System() {
            stdin = App.inputStream
            stdout = App.outputStream
            stderr = App.errorStream
            args = App.args
            //  TODO MOB
            env = App.env
            fs = new FileSystem("/")
            log = App.log
            platform = Config.title
            this.global = global
        }

    /*
        function print(...args): Void
            global.print(...args)
     */
        function log(...msgs): Void
            App.logger.info(...msgs)

        platform
    }

    var system = new System
}
