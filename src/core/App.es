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
        Application configuration state. The App class is a singleton class object and that accesses and controls 
        the applications execution environment including the current working directory, application command line 
        arguments, environment strings, path to the application's executable and input and output streams.
        @spec ejs
        @stability evolving
     */
    enumerable class App {

        use default namespace public

        /** 
            Configuration environment specified in the ~/.ejsrc or ejsrc configuration files.
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
            log: {
                enable: true,
                location: "stderr",
                level: 0,
            },
            app: {
                reload: false,
            },
            cache: {
                app: { enable: false },
            },
            dirs: {
                cache: Path("cache"),
            },
            files: {
                ejsrc: Path("ejsrc"),
            },
            init: { },
        }

        /**
            Default event Emitter for the application.
         */
        static var emitter: Emitter = new Emitter

        /** 
            Application logger. This singleton object respresents the Application default logger.
            If the ejsrc startup configuration file defines a log.location field, the log logger will send messages to
            the defined location. Otherwise, messages will be sent to the MprLog stream. 
         */
        public static var log: Logger

        /** 
            Application MPR log object. This singleton object represents the Application log file specified via the
            --log command line switch.
         */
        public static var mprLog: MprLog

        /** 
            Application name. Single word, lower-case name for the application. This is initialized to the name of
            the script or "ejs" if running interactively.
         */
        static var name: String

        /**
            Application in-memory cache reference
         */
        static var cache: Cache

        /**
            Application start time
         */
        static var started: Date = new Date

        /**
            Test object for unit tests when run via utest
         */
        static var test 

        /** 
            Application title name. Multi-word, Camel Case name for the application suitable for display. This is 
            initialized to the name of the script or "Embedthis Ejscript" if running interactively.
         */
        static var title: String

        /** 
            Application version string. Set to a version string [format Major.Minor.Patch-Build]. For example: 1.1.2-3.
            Initialized to the ejs version.
         */
        static var version: String

        /** 
            Application command line arguments. Set to an array containing each of the arguments. If the ejs command 
                is invoked as "ejs script arg1 arg2", then args[0] will be "script", args[1] will be "arg1" etc.
         */
        native static function get args(): Array

        /** 
            Change the application's working directory
            @param value The path to the new working directory
         */
        native static function chdir(value: Object): Void

        /** 
            Create a search path array for locating ejs modules. This converts a delimited PATH to an array of 
            paths suitable for use by the $search property. NOTE: this does not modify the application's search path.
            @param searchPath String containing a colon separated (or semi-colon on Windows) set of paths.
                If search path is null, the default system search paths are returned.
            @return An array of search paths.
         */
        native static function createSearch(searchPath: String? = null): Array

        /** 
            The application's current directory
            @return the path to the current directory
         */
        native static function get dir(): Path

        /** 
            Application environment. Object hash of all environment variables.
         */
        native static function get env(): Object

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
            @param how How the exit should proceed. Options are: "normal", "immediate" or "graceful". A normal exit
                will flush buffered data and close files and resources and then exit without waiting.  
                An immediate exit will exit without writing buffered data or closing files. A graceful exit will wait
                until the system is idle and then do a normal exit. A system is idle when it has no running commands, 
                sockets, Http requests or worker threads.
         */
        native static function exit(status: Number = 0, how: String = "normal"): Void

        /** 
            Get an environment variable.
            @param name The name of the environment variable to retrieve.
            @return The value of the environment variable or null if not found.
         */
        native static function getenv(name: String): String

        /**
            The group ID of the user account running the application. Only supported on Unix.
         */
        native static function get gid(): Number
        
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
                    blend(App.config, path.readJSON(), {overwrite: overwrite})
                } catch (e) {
                    errorStream.write(App.exePath.basename +  " Can't parse " + path + ": " + e + "\n")
                }
            }
        }

        //  TODO - move to locale
        /** 
            Get the current language locale for this application
            @hide
         */
        # FUTURE
        native static function get locale(): String
        # FUTURE
        native static function set locale(locale: String): Void

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
            The process ID of the application
         */
        native static function get pid(): Number
        
        /** 
            Update an environment variable.
            @param name The name of the environment variable to retrieve.
            @param value The new value to define for the variable.
         */
        native static function putenv(name: String, value: String): Void

        /** 
            Run the application event loop. 
            A script may call run() to service events. Calling run() will cause the ejs shell to wait and service 
            events until instructed to exit via App.exit. If the application is hosted in a web server, this routine will
            return true immediately without blocking. 
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately. Set to -1 for no timeout.
            @param oneEvent If true, return immediately after servicing at least one ejs event.
            @return True if an event happened. Otherwise return false if the timeout expired before any event.
         */
        native static function run(timeout: Number = -1, oneEvent: Boolean = false): Boolean

        /** 
            The current module search path. Set to an array of Paths.
         */
        native static function get search(): Array
        native static function set search(paths: Array): Void

        /** 
            Sleep the application for the given number of milliseconds. Events will be serviced while asleep.
            An alternative to sleep is $App.run which can be configured to sleep and return early if an event is received.
            @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
         */
        native static function sleep(delay: Number = -1): Void

        /** 
            The current module search path . Set to a delimited searchPath string. Warning: This will be changed to an
            array of paths in a future release.
            @stability deprecated.
            @deprecate 1.0.0
            @hide
         */
        # Config.Legacy
        static function get searchPath(): String {
            if (Config.OS == "WIN") {
                return search.join(";")
            } else {
                return search.join(":")
            }
        }

        # Config.Legacy
        static function set searchPath(path: String): Void {
            if (Config.OS == "WIN") {
                search = path.split(";")
            } else {
                search = path.split(":")
            }
        }

        /**
            Service events
            @param count Count of events to service. Defaults to unlimited.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @stability deprecated
            @hide
         */
        # Config.Legacy
        static function serviceEvents(count: Number = -1, timeout: Number = -1): Void {
            if (count < 0) {
                run(timeout, false)
            } else {
                for (i in count) {
                    run(timeout, true)
                }
            }
        }

        /**
            The user ID of the user account running the application. Only supported on Unix.
         */
        native static function get uid(): Number
        
        /**
            Redirect the Application's logger based on the App.config.log setting
            Ignored if app is invoked with --log on the command line.
         */
        static function updateLog(): Void {
            let log = config.log
            if (log && log.enable && !App.mprLog.fixed) {
                App.log.redirect(log.location, log.level)
                App.mprLog.redirect(log.location, log.level)
            }
        }

        /** 
            @hide
            Wait for an event
            @param Observable object
            @param events Events to consider
            @param timeout Timeout in milliseconds
         */
        static function waitForEvent(obj: *, events: Object, timeout: Number = Number.MaxInt32): Boolean {
            let done
            function callback(event) done = true
            obj.on(events, callback)
            for (let mark = new Date; !done && mark.elapsed < timeout; )
                App.run(timeout - mark.elapsed, true)
            obj.off(events, callback)
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
        App.version = Config.Version
        App.mprLog = new MprLog

        /*  
            Load ~/.ejsrc and ejsrc
         */
        let config = App.config = {}
        let dir = App.getenv("HOME")
        if (dir) {
            App.loadrc(Path(dir).join(".ejsrc"))
        }
        App.loadrc("ejsrc")
        blend(config, App.defaultConfig, {overwrite: false})

        stdout = TextStream(App.outputStream)
        stderr = TextStream(App.errorStream)
        stdin = TextStream(App.inputStream)

        let log = config.log
        let stream
        if (log.enable) {
            let level = log.level
            let location = log.location
            if (App.mprLog.fixed) {
                /* App invoked with a --log switch */
                level = App.mprLog.level
                location = App.mprLog;
            } else {
                App.mprLog.redirect(location, level)
            }
            App.log = new Logger(App.name, location, level)
            if (log.match) {
                App.log.match = log.match
            }
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
        if (config.cache) {
            //  MOB - should there be a config.cache.enable instead
            App.cache = new Cache(null, blend({shared: true}, config.cache))
        }
    }

    appInit()
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
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
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
