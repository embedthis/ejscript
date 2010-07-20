/*
    Logger.es - Log file control class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB -- should this be a stream?
    /** 
        Logger objects provide a convenient and consistent method to capture and store logging information.
        Loggers can direct output to streams and may be aggregated in a hierarchical manner. The verbosity of 
        logging can be managed and messages can be filtered.
   
        A logger may have a "parent" logger in order to create hierarchies of loggers for granular control of logging.
        For example, a logger can be created for each class in a package with all such loggers having a single parent. 
        Loggers can send log messages to their parent and inherit their parent's log level. 
        The top level logger for an application is defined by App.log.
 
        Loggers may define a filter function that returns true or false depending on whether a specific message 
        should be logged or not. A matching pattern can alternatively be used to filter messages based on the logger name.

        Loggers are themselves Streams and Stream filters can be stacked atop Loggers.
        @spec ejs
        @stability prototype
     */
    class Logger implements Stream {

        use default namespace public

        /** 
            Logging level for no logging.
         */
        static const Off: Number = -1 

        /** 
            Logging level for most serious errors.
         */
        static const Error: Number = 0

        /** 
            Logging level for warnings.
         */
        static const Warn: Number = 1

        /** 
            Logging level for informational messages.
         */
        static const Info: Number = 2

        /** 
            Logging level for configuration output.
         */
        static const Config: Number = 3

        /** 
            Logging level to output all messages.
         */
        static const All: Number = 9

        private var _filter: Function
        private var _level: Number = 0
        private var _pattern: RegExp
        private var _name: String

        private var _outStream: Stream

        /** 
            Logger constructor.
            The Logger constructor can create different types of loggers based on the three (optional) arguments. 
            @param name Unique name of the logger. Loggers are typically named after the module, class or subsystem they 
            are associated with.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering.
            @param level Optional integer verbosity level. Messages with a message level less than or equal to the defined
                logger level will be emitted. Range is 0 (least verbose) to 9.
            @example:
                var file = File("progress.log", "w")
                var log = new Logger("name", file, 5)
                log.debug(2, "message")
         */
        function Logger(name: String, location, level: Number? = 0) {
            _level = level
            redirect(location)
            _name = (_outStream is Logger) ? (_outStream.name + "." + name) : name 
        }

        /** 
            Redirect log output.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering.
         */
        function redirect(location): Void {
            if (location is Stream) {
                _outStream = location
            } else {
                location = location.toString()
                let [path, level] = location.split(":")
                if (level) {
                    _level = level
                }
                if (path == "stdout") {
                    _outStream = App.outputStream
                } else if (path == "stderr") {
                    _outStream = App.errorStream
                } else {
                    _outStream = File(path).open("w")
                }
            }
        }

        /** 
            Sync/async mode. Not supported for Loggers.
            @hide
         */
        function get async(): Boolean
            false

        function set async(enable: Boolean): Void {
            throw "Async mode not supported"
        }

        /** 
            Close the logger 
         */
        function close(): Void
            _outStream = null

        /** 
            Filter function for this logger. The filter function is called with the following signature:
            with "this" set to the Logger instance. The $log parameter is set to the original logger that created the
            message.
            function filter(log: Logger, name: String, level: Number, kind: String, msg: String): Boolean
            @param fn The filter function must return true or false.
         */
        function get filter(): Function
            _filter

        function set filter(fn: Function): void
            _filter = fn

        /**
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (_outStream_) {
                _outStream.flush(dir)
            }
        }

        /** 
            The numeric verbosity setting (0-9) of this logger. Zero is least verbose, nine is the most verbose.
         */
        function get level(): Number
            _level

        function set level(level: Number): void
            _level = level

        /** 
            Matching expression to filter log messages. The match regular expression is used to match 
            against the Logger names.
         */
        function get match(): RegExp
            _pattern

        function set match(pattern: RegExp): void 
            _pattern = pattern

        /**
            Get the MPR log level via a command line "--log spec" switch
            @hide
         */
        static native function get mprLogLevel(): Number

        /**
            MPR log file defined via a command line "--log spec" switch
            @hide
         */
        static native function get mprLogFile(): Stream

        /** 
            The name of this logger.
         */
        function get name(): String
            _name

        function set name(name: String): void
            _name = name

        /** 
            The output stream used by the logger.
         */
        function get outStream(): Stream
            _outStream

        function set outStream(stream: Stream): void
            _outStream = stream

        /** 
            Emit a debug message. The message level will be compared to the logger setting to determine 
            whether it will be output to the devices or not. Also, if the logger has a filter function set that 
            may filter the message out before logging.
            @param level The level of the message.
            @param msgs The string message to log.
         */
        function debug(level: Number, ...msgs): void 
            emit("", level, "", msgs.join(" ") + "\n")

        /** 
            Emit a configuration message.
            @param msgs Data to log.
         */
        function config(...msgs): void
            emit("", Config, "CONFIG", msgs.join(" ") + "\n")

        /** 
            Emit an error message.
            @param msgs Data to log.
         */
        function error(...msgs): void
            emit("", Error, "ERROR", msgs.join(" ") + "\n")

        /** 
            Emit an informational message.
            @param msgs Data to log.
         */
        function info(...msgs): void
            emit("", Info, "INFO", msgs.join(" ") + "\n")

        /** 
            Emit an activity message
            @param tag Activity tag to prefix the message. The tag string is wraped in "[]".
            @param args Output string to log
            @hide
            @stability prototype
         */
        function activity(tag: String, ...args): Void {
            let msg = args.join(" ")
            let msg = "%12s %s" % (["[" + tag.toUpper() + "]"] + [msg]) + "\n"
            if (_level > 0) {
                write(msg)
            }
        }

        /** 
            @hide
         */
        function observe(name, observer: Function): Void {
            throw "observe is not supported"
        }

        /** 
            @hide
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number  {
            throw "Read not supported"
            return null
        }

        /** 
            @hide
         */
        function removeObserver(name, observer: Function): Void {
            throw "observe is not supported"
        }

        /** 
            Write raw data to the logger stream.
            @duplicate Stream.write
         */
        function write(...data): Number
            (_outStream) ? _outStream.write(data.join(" ")) : 0

        /** 
            Emit a warning message.
            @param msgs The data to log.
         */
        function warn(...msgs): void
            emit("", Warn, "WARN", msgs.join(" ") + "\n")

        /* 
            Emit a message. The message level will be compared to the logger setting to determine whether it will be 
            output to the devices or not. Also, if the logger has a filter function set that may filter the message 
            out before logging.
            @param origin Name of the logger that originated the message
            @param level The level of the message.
            @param kind Message kind (debug, info, warn, error)
            @param msg The string message to emit
         */
        private function emit(origin: String, level: Number, kind: String, msg: String): Void {
            origin ||= _name
            if (level > _level || !_outStream) {
                return
            }
            if (_pattern && !origin.match(_pattern)) {
                return
            }
            if (_filter && !filter(this, origin, level, kind, msg))
                return
            if (_outStream is Logger) {
                _outStream.emit(origin, level, kind, msg)
            } else {
                if (kind)
                    _outStream.write(origin + ": " + kind + ": " + msg)
                else _outStream.write(origin + ": " + level + ": " + msg)
            }
        }
    }
}
