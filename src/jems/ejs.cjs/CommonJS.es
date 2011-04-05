/*
    CommonJS class
 */

module ejs.cjs {

    /**
        System for CommonJS. This is the base class for "system"
     */
    enumerable dynamic class CommonSystem {
        use default namespace public

        static var stdin: Stream
        static var stdout: Stream
        static var stderr: Stream
        static var args: Array
        static var env: Object
        static var fs: CommonFile
        static var platform: String
        static var system: CommonSystem

        /** @hide */
        function CommonSystem() {
            stdin = App.inputStream
            stdout = App.outputStream
            stderr = App.errorStream
            args = App.args
            env = {}        // App.env
            platform = Config.title
            this.global = global

            fs = new CommonFile
            log = App.log
            system = this
        }

    /*
        function print(...args): Void
            global.print(...args)
     */
        function log(...msgs): Void
            App.logger.info(...msgs)
    }

    class CommonFile {
        use default namespace public

        //  MOB -- options differ
        function open(path: String, options): Stream
            File(path, options)

        function read(path: String, options): String
            Path(path).readString()

        function basename(path: String, extension: String = ""): String
            Path(path).basename

        function write(path: String, data): Void
            Path(path).write(data)
    }

    global.system = new CommonSystem
}
