/*
    App.es -- Application configuration and control.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Standard error text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stderr: TextStream

    /** 
        Standard input text stream. Use read(), readLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdin: TextStream

    /** 
        Standard output text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdout: TextStream

    /** 
        Application configuration state. The App class is a singleton class object. The App class is accessed via
        the App global type object. It provides  methods to interrogate and control the applications environment including
        the current working directory, application command line arguments, path to the application's executable and
        input and output streams.
        @spec ejs
        @stability evolving
     */
    class App {

        use default namespace public

        /** 
            Configuration environment specified in the .ejsrc configuration files.
         */
        static var config: Object

        /*  
            Standard I/O streams. These can be any kind of stream.
         */
        private static var _errorStream: Stream
        private static var _inputStream: Stream
        private static var _outputStream: Stream

        static internal var defaultConfig = {
            /*
                search: null,
                script: null,
             */
            log: {
                enable: true,
                where: "stdout",
                level: 2,
                /*
                    match: null,
                 */
            },
            cache: {
                enable: true,
            },
            directories: {
                cache: "cache",
            }
        }

        /** 
            Application command line arguments. Set to an array containing each of the arguments. If the ejs command 
                is invoked as "ejs script arg1 arg2", then args[0] will be "script", args[1] will be "arg1" etc.
         */
        native static function get args(): Array

        /** 
            The application's current directory
            @return the path to the current directory
         */
        native static function get dir(): Path

        /** 
            Change the application's Working directory
            @param value The path to the new working directory
         */
        native static function chdir(value: Object): Void

        /** 
            The directory containing the application executable
         */
        native static function get exeDir(): Path

        /** 
            The application executable path
         */
        native static function get exePath(): Path

        /** 
            The application's standard error file stream
         */
        static function get errorStream(): Stream
            _errorStream

        /** 
            Set the standard error stream. Changing the error stream will close and reopen stderr.
            @param stream The output stream.
         */
        static function set errorStream(stream: Stream): Void {
            _errorStream = stream
            if (stderr) {
                stderr.close()
            }
            stderr = TextStream(_errorStream)
        }

        /** 
            Stop the program and exit.
            @param status The optional exit code to provide the environment. If running inside the ejs command program,
                the status is used as process exit status.
         */
        native static function exit(status: Number = 0): Void

        /** 
            Get an environment variable.
            @param name The name of the environment variable to retrieve.
            @return The value of the environment variable or null if not found.
         */
        native static function getenv(name: String): String

        /** 
            The application's standard input file stream
         */
        static function get inputStream(): Stream
            _inputStream

        /** 
            Set the standard input stream. Changing the input stream will close and reopen stdin.
            @param stream The input stream.
         */
       static function set inputStream(stream: Stream): Void {
            _inputStream = stream
            if (stdin) {
                stdin.close()
            }
            stdin = TextStream(_inputStream)
        }
        
        //  TODO - move to locale
        /** 
            Get the current language locale for this application
            @hide
         */
        # FUTURE
        native static function get locale(): String

        //  TODO - move to a Locale class
        /** 
            Set the current language locale
         */
        # FUTURE
        native static function set locale(locale: String): Void

        /** 
            Set the current logger
         */
        public static var logger: Logger

        /** 
            Application name. Set to a single word, lower-case name for the application.
         */
        static function get name(): String
            Config.Product

        //  TODO need a better name than noexit, TODO could add a max delay option.
        /** 
            Control whether an application will exit when global scripts have completed. Setting this to true will cause
            the application to continue servicing events until the $exit method is explicitly called. The default 
            application setting of noexit is false.
            @param exit If true, the application will exit when the last script completes.
         */
        native static function noexit(exit: Boolean = true): Void

        /** 
            The application's standard output file stream
         */
        static function get outputStream(): Stream
            _outputStream

        /** 
            Set the standard output stream. Changing the output stream will close and reopen stdout.
            @param stream The output stream.
         */
        static function set outputStream(stream: Stream): Void {
            _outputStream = stream
            if (stdout) {
                stdout.close()
            }
            stdout = TextStream(_outputStream)
        }
        
        /** 
            Update an environment variable.
            @param name The name of the environment variable to retrieve.
            @param value The new value to define for the variable.
         */
        native static function putenv(name: String, value: String): Void

        /** 
            The current module search path. Set to an array of Paths.
         */
        native static function get search(): Array

        /** 
            Set the current module search path
            @parram paths An array of paths
         */
        native static function set search(paths: Array): Void

        /** 
            Create a search path array.
            @param searchPath String containing a colon separated (or semi-colon on Windows) set of paths.
                If search path is null, the default system search paths are returned
            @return An array of search paths.
         */
        native static function createSearch(searchPath: String? = null): Array

        /** 
            Service events
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @param oneEvent If true, return immediately after servicing one event.
         */
        native static function serviceEvents(timeout: Number = -1, oneEvent: Boolean = false): Void

        /** 
            Set an environment variable.
            @param env The name of the environment variable to set.
            @param value The new value.
            @return True if the environment variable was successfully set.
         */
        # FUTURE
        native static function setEnv(name: String, value: String): Boolean

        /** 
            Sleep the application for the given number of milliseconds
            @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
         */
        native static function sleep(delay: Number = -1): Void

        /** 
            Application title name. Multi-word, Camel Case name for the application suitable for display.
         */
        static static function get title(): String
            Config.Title

        /** 
            Application version string. Set to a version string of the format Major.Minor.Patch-Build. For example: 1.1.2-3.
         */
        static static function get version(): String
            Config.Version
    }

    /**  
        Initialize ejs applications. This is invoked when this module loads
        @hide
     */
    function appInit(): Void {
        /*  
            Load ~/.ejsrc
         */
        let dir = App.getenv("HOME")
        if (dir) {
            let path = Path(dir).join(".ejsrc")
            if (path.exists) {
                try {
                    App.config = deserialize(path.readString())
                } catch (e) {
                    error("Can't parse " + path + ": " + e)
                }
            }
        }
        let config = App.config || {}
        App.config = config

        blend(config, App.defaultConfig, false)

        stdout = TextStream(App.outputStream)
        stderr = TextStream(App.errorStream)
        stdin = TextStream(App.inputStream)

        let log = config.log
        if (log.enable) {
            let stream
            if (log.where == "stdout") {
                stream = App.outputStream
            } else if (log.where == "stderr") {
                stream = App.errorStream
            } else {
                stream = File(log.where, "w")
            }
            App.logger = new Logger(App.name, stream, log.level)
            if (log.match) {
                App.logger.match = log.match
            }
        }

        /*  
            Append search paths
         */
        if (config.search) {
            if (config.search is Array) {
                App.search = config.search + App.search
            } else if (config.search is String) {
                App.search = config.search.split(Path.SearchSeparator) + App.search
            }
        }

        /*  
            Run load scripts
         */
        if (config.scripts) {
            for each (m in config.scripts) {
                load(m)
            }
        }
    }

    appInit()
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
