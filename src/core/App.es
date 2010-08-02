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
    enumerable class App {

        use default namespace public

        /** 
            Configuration environment specified in the .ejsrc/ejsrc configuration files.
         */
        static var config: Object

        /**
            Separator string to use when constructing PATH style search strings
         */
        static var SearchSeparator: String = (Config.OS == "WIN") ? ";" : ":"

        /*  
            Standard I/O streams. These can be any kind of stream.
         */
        private static var _errorStream: Stream
        private static var _inputStream: Stream
        private static var _outputStream: Stream

        /**
            Default ejsrc configuration for the application
            @hide
         */
        static internal var defaultConfig = {
            /*
                search: null,
                script: null,
             */
            log: {
                enable: true,
                location: "stderr",
                level: 0,
                /* match: null, */
            },
            cache: {
                enable: false,
                reload: true,
            },
            directories: {
                cache: "cache",
            },
            init: {
                /* load: [] */
            },
            test: {
            },
        }

        /**
            Default event Emitter for the application
         */
        static var emitter: Emitter = new Emitter

        /** 
            Default application logger. This is set to stderr unless the program specifies an output log via the --log 
            command line switch.
         */
        public static var log: Logger

        /** 
            Application name. Set to a single word, lower-case name for the application.
         */
        static var name: String

        /** 
            Application title name. Multi-word, Camel Case name for the application suitable for display.
         */
        static var title: String

        /** 
            Application version string. Set to a version string of the format Major.Minor.Patch-Build. For example: 1.1.2-3.
         */
        static var version: String

        /** 
            Application command line arguments. Set to an array containing each of the arguments. If the ejs command 
                is invoked as "ejs script arg1 arg2", then args[0] will be "script", args[1] will be "arg1" etc.
         */
        native static function get args(): Array

        /** 
            Create a search path array.
            @param searchPath String containing a colon separated (or semi-colon on Windows) set of paths.
                If search path is null, the default system search paths are returned
            @return An array of search paths.
         */
        native static function createSearch(searchPath: String? = null): Array

//  MOB -- have 2 routines; doEvents, doOneEvent
        /** 
            Run the event loop. This is typically done automatically by the hosting program and is not normally required
                in user programs.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @param oneEvent If true, return immediately after servicing one event.
         */
        native static function eventLoop(timeout: Number = -1, oneEvent: Boolean = false): Void

        /** 
            Change the application's Working directory
            @param value The path to the new working directory
         */
        native static function chdir(value: Object): Void

        /** 
            The application's current directory
            @return the path to the current directory
         */
        native static function get dir(): Path


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
            Set the standard error stream. Changing the error stream will close and reopen stderr.
         */
        static function get errorStream(): Stream
            _errorStream

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

//  MOB -- need get env()
        /** 
            Get an environment variable.
            @param name The name of the environment variable to retrieve.
            @return The value of the environment variable or null if not found.
         */
        native static function getenv(name: String): String

        /** 
            Set the standard input stream. Changing the input stream will close and reopen stdin.
         */
        static function get inputStream(): Stream
            _inputStream

       static function set inputStream(stream: Stream): Void {
            _inputStream = stream
            if (stdin) {
                stdin.close()
            }
            stdin = TextStream(_inputStream)
        }
        
        /**
            Load an "ejsrc" configuration file
            This loads an Ejscript configuration file and blends the contents with App.config. 
            @param path Path name of the file to load
            @param overwrite If true, then new configuration values overwrite existing values in App.config.
         */
        static function loadrc(path: Path, overwrite: Boolean = true) {
            if (path.exists) {
                try {
                    blend(App.config, path.readJSON(), overwrite)
                } catch (e) {
                    errorStream.write(App.exePath.basename +  " Can't parse " + path + ": " + e + "\n")
                }
            }
        }

        //  MOB TODO - move to locale
        /** 
            Get the current language locale for this application
            @hide
         */
        # FUTURE
        native static function get locale(): String
        # FUTURE
        native static function set locale(locale: String): Void

        //  MOB TODO need a better name than noexit, TODO could add a max delay option.
        /** 
            Control whether an application will exit when global scripts have completed. Setting this to true will cause
            the application to continue servicing events until the $exit method is explicitly called. The default 
            application setting of noexit is false.
            @param exit If true, the application will exit when the last script completes.
         */
        native static function noexit(exit: Boolean = true): Void

        /** 
            The standard output stream. Changing the output stream will close and reopen stdout.
         */
        static function get outputStream(): Stream
            _outputStream

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
        native static function set search(paths: Array): Void

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

        //  DEPRECATED
        /** 
            The current module search path . Set to a delimited searchPath string. Warning: This will be changed to an
            array of paths in a future release.
            @stability deprecated.
            @deprecate
            @hide
         */
        static function get searchPath(): String {
            if (Config.OS == "WIN") {
                return search.join(";")
            } else {
                return search.join(":")
            }
        }
        static function set searchPath(path: String): Void {
            if (Config.OS == "WIN") {
                search = path.split(";")
            } else {
                search = path.split(":")
            }
        }

        //  DEPRECATED
        /**
            Service events
            @param count Count of events to service. Defaults to unlimited.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @stability deprecated
            @hide
         */
        static function serviceEvents(count: Number = -1, timeout: Number = -1): Void {
            if (count < 0) {
                eventLoop(timeout, false)
            } else {
                for (i in count) {
                    eventLoop(timeout, true)
                }
            }
        }

        /** @hide
            Wait for an event
            @param Observable object
            @param events Events to consider
            @param timeout Timeout in milliseconds
         */
        static function waitForEvent(obj: *, events: Object, timeout: Number = Number.MaxInt32): Boolean {
            let done
            function callback(event) done = true
            obj.observe(events, callback)
            for (let mark = new Date; !done && mark.elapsed < timeout; )
                App.eventLoop(timeout - mark.elapsed, 1)
            obj.removeObserver(events, callback)
            return done
        }
    }

    /**  
        Initialize ejs applications. This is invoked when this module loads
        @hide
     */
    function appInit(): Void {
        App.name = App.args[0] || Config.Product
        App.title = App.args[0] || Config.Title

        /*  
            Load ~/.ejsrc and ejsrc
         */
        let config = App.config = {}
        let dir = App.getenv("HOME")
        if (dir) {
            App.loadrc(Path(dir).join(".ejsrc"))
        }
        App.loadrc("ejsrc")
        blend(config, App.defaultConfig, false)

        stdout = TextStream(App.outputStream)
        stderr = TextStream(App.errorStream)
        stdin = TextStream(App.inputStream)

        let log = config.log
        if (log.enable) {
            let stream = Logger.nativeStream
            if (stream) {
                log.level = Logger.nativeLevel
            } else if (log.location == "stdout") {
                stream = App.outputStream
            } else if (log.location == "stderr") {
                stream = App.errorStream
            } else {
                stream = File(log.location, "w")
            }
            App.log = new Logger(App.name, stream, log.level)
            if (log.match) {
                App.log.match = log.match
            }
            Logger.nativeLevel = log.level
        }

        /*  Append search paths */
        if (config.search) {
            if (config.search is Array) {
                App.search = config.search + App.search
            } else if (config.search is String) {
                App.search = config.search.split(App.SearchSeparator) + App.search
            }
        }

        /*  Pre-load modules and scripts */
        if (config.init.load) {
            for each (m in config.init.load) {
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
