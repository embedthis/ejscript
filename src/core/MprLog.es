/*
    MprLog.es - Application Log File class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The MprLog class manages the Application's internal MPR log mechanism. If the Application was started with a 
        "--log" command line switch, the application will write log messages to the specified file, otherwise 
        messages will be sent to the standard error console. The Application will have a single instance of the 
        MprLog class created and stored in the App.mprLog property.

        The Logger class can be used to create higher level logging filters and aggregators and use the MprLog as the
        final output stream.

        @spec ejs
        @stability prototype
     */
    class MprLog implements Stream {

        use default namespace public

        /** 
            Sync/async mode. Not supported.
            @hide
         */
        function get async(): Boolean
            false

        function set async(enable: Boolean): Void {
            throw "Async mode not supported"
        }

        /** 
            Close the MprLog and stop logging
            @hide
         */
        function close(): Void {}

        /** 
            Emit messages to the MprLog stream at a given level
            @param level Verbosity level at which to emit the message (0-9).
            @param data Data messages to emit
         */
        native function emit(level: Number, ...data): Number

        /**
            Is the MPR logging fixed. Set to true if the application logging has been fixed by a hosting environment or
            if the application initiated logging via a command line --log switch.
         */
        native function get fixed(): Boolean
        native function set fixed(on: Boolean): Void

        /**
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void { }

        /** 
            The numeric verbosity setting (0-9) of this MprLog. Zero is least verbose, nine is the most verbose.
            Messages with a lower (or equal) verbosity level than the MprLog's level are emitted.
            WARNING: Changing the logging verbosity level will affect logging for all interpreters.
         */
        native function get level(): Number
        native function set level(level: Number): void 

        /** @hide */
        function off(name, observer: Function): Void {
            throw "off is not supported"
        }

        /** @hide */
        function on(name, observer: Function): Void {
            throw "on is not supported"
        }

        /** @hide */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? {
            throw "Read not supported"
            return null
        }

        /** 
            Redirect log output. WARNING: Redirecting the logging output will redirect messages for all interpreters.
            @param location Output location to send messages to.
            @param level Verbosity level for logging. 
         */
        native function redirect(location: String, level: Number? = null): Void

        /** 
            Write messages to the MprLog stream at level 0
            @duplicate Stream.write
         */
        function write(...data): Number
            emit(0, ...data)
    }
}
