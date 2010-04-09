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
        The top level logger for an application is defined by App.logger.
 
        Loggers may define a filter function that returns true or false depending on whether a specific message 
        should be logged or not. A matching pattern can alternatively be used to filter messages based on the logger name.
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
        private var _outstream: Stream
        private var _parent: Logger

        /** 
            Logger constructor.
            The Logger constructor can create different types of loggers based on the three (optional) arguments. 
            @param name Unique name of the logger. Loggers are typically named after the module, class or subsystem they 
            are associated with.
            @param output Optional output device or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering. The default output device is defined 
                by the application, typically via a "--log" command line switch. 
            @param level Optional integer verbosity level. Messages with a message level less than or equal to the defined
                logger level will be emitted. Range is 0 (least verbose) to 9.
            @example:
                var file = File("progress.log", "w")
                var logger = new Logger("name", file, 5)
                logger.log(2, "message")
         */
        function Logger(name: String, output = null, level: Number? = 0) {
            _name = name
            if (output && output is Logger) {
                _level = output.level
                _parent = output
            } else {
                if (output == "stdout") {
                    _outstream = App.outputStream
                } else if (output == "stderr") {
                    _outstream = App.errorStream
                } else {
                    _outstream = output || App.errorStream
                }
                _level = level
            }
        }

        /** 
            @hide
         */
        function addListener(name, listener: Function): Void {
            throw "addListener is not supported"
        }

        /** 
            The current sync/async mode.
            @hide
         */
        function get async(): Boolean
            false

        /** 
            Set the current sync/async mode. The async mode affects the blocking APIs: close, read and write.
            If in async mode, all Stream calls will not block. If listeners have been registered, they can be used to
            respond to events to interface with the stream.
            @param enable If true, set the stream into async mode
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw "Async mode not supported"
            }
        }

        /** 
            Close the logger 
         */
        function close(): Void {
            _outstream = null
        }

        /** 
            The filter function in use by the logger.
         */
        function get filter(): Function
            _filter

        /** 
            Set the filter function for this logger. The filter function is called with the following signature:
            with "this" set to the Logger instance. The $log parameter is set to the original logger that created the
            message.
            function filter(log: Logger, level: Number, msg: String): Boolean
            @param fn The filter function must return true or false.
         */
        function set filter(fn: Function): void {
            _filter = fn
        }

        /**
            @hide
         */
        function flush(): Void {}

        /** 
            The numeric verbosity setting (0-9) of this logger.
         */
        function get level(): Number
            _level

        /** 
            Set the output level of this logger. (And all child loggers who have their logging level set to Inherit.)
            @param level The next logging level (verbosity).
         */
        function set level(level: Number): void {
            _level = level
        }

        /** 
            The match pattern for the logger.
         */
        function get match(): RegExp
            _pattern

        /** 
            Set a matching expression. The match expression is used to match against the Logger names.
            @param pattern Regular expression.
         */
        function set match(pattern: RegExp): void {
            _pattern = pattern
        }

        /** 
            The name of this logger.
         */
        function get name(): String
            _name

        /** 
            Set the name for this logger.
            @param name An optional string name.
         */
        function set name(name: String): void {
            _name = name
        }

        /** 
            The output stream used by the logger.
         */
        function get outstream(): Stream
            _outstream

        /** 
            Set the output stream device for this logger.
            @param stream New output stream for the logger
         */
        function set outstream(stream: Stream): void {
            _outstream = outstream
        }

        /** 
            The parent of this logger.
         */
        function get parent(): Logger
            _parent

        /** 
            Set the parent logger for this logger.
            @param parent A logger.
         */
        function set parent(parent: Logger): void {
            _parent = parent
        }

        /** 
            Emit a debug message. The message level will be compared to the logger setting to determine 
            whether it will be output to the devices or not. Also, if the logger has a filter function set that 
            may filter the message out before logging.
            @param level The level of the message.
            @param msgs The string message to log.
         */
        function debug(level: Number, ...msgs): void 
            emit(this, level, "", "", msgs.join(" ") + "\n")

        /** 
            Emit a configuration message.
            @param msgs Strings to log.
         */
        function config(...msgs): void
            emit(this, Config, "", "CONFIG", msgs.join(" ") + "\n")

        /** 
            Emit an error message.
            @param msgs Strings to log.
         */
        function error(...msgs): void
            emit(this, Error, "", "ERROR", msgs.join(" ") + "\n")

        /** 
            Emit an informational message.
            @param msgs Strings to log.
         */
        function info(...msgs): void
            emit(this, Info, "", "INFO", msgs.join(" ") + "\n")

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
        function removeListener(name, listener: Function): Void {
            throw "addListener is not supported"
        }

        /** 
            @duplicate Stream.write
            Write informational data to logger
         */
        function write(...data): Number
            info(data)

        /** 
            Emit a warning message.
            @param msgs The strings to log.
         */
        function warn(...msgs): void
            emit(this, Warn, "", "WARN", msgs.join(" ") + "\n")

        /* 
            Emit a message. The message level will be compared to the logger setting to determine whether it will be 
            output to the devices or not. Also, if the logger has a filter function set that may filter the message 
            out before logging.
            @param log Logger to write to
            @param level The level of the message.
            @param name Name tag to append to the message
            @param kind Message kind (debug, info, warn, error)
            @param msg The string message to emit
         */
        private function emit(log: Logger, level: Number, name: String, kind: String, msg: String): Void {
            if (level > _level || !_outstream)
                return
            if (name)
                name = _name + "." + name
            else name = _name
            if (_pattern && !name.match(_pattern))
                return
            if (_filter && !filter.call(this, log, level, msg))
                return
            if (_parent) {
                _parent.emit(log, level, name, kind, msg)
            } else {
                if (kind)
                    _outstream.write(name + ": " + kind + ": " + msg)
                else _outstream.write(name + ": " + msg)
            }
        }
    }
}
