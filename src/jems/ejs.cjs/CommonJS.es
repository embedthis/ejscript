/*
    CommonJS class
 */

module ejs.cjs {

    /**
        CommonJS System class. This is provided for compatibility with the CommonJS environment.
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
        /** @hide */
        function log(...msgs): Void
            App.logger.info(...msgs)
    }

    /**
        CommonJS File class. This is provided for compatibility with the CommonJS environment.
     */
    class CommonFile {
        use default namespace public

        /**
            Open a path and return a File object.
            @param path Filename to open
            @param options Open options
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return a File stream object which implements the Stream interface.
            @throws IOError if the path or file cannot be created.
         */
        function open(path: String, options): Stream
            File(path, options)

        /**
            Read a file and return a string
            @param path Filename to read from
            @param options Ignored
            @return File data as a string
         */
        function read(path: String, options = null): String
            Path(path).readString()

       /**
            The base of portion of the path. The base portion is the trailing portion without any directory elements.
            @param path Path name to examine
            @param extension Ignored
            @return the base portion of the path as a string
        */
        function basename(path: String, extension: String = ""): String
            Path(path).basename

        /**
            Write data to a file.
            @param path Filename to write to
            @param data Data to write to the file
         */
        function write(path: String, data): Void
            Path(path).write(data)
    }

    global.system = new CommonSystem
}
