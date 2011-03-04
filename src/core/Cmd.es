/*
    Cmd.es - Cmd class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Cmd class supports invoking other programs on the same system. 
        @spec ejs
     */
    class Cmd implements Stream {

        use default namespace public

        /**
            Cached command output
            @hide
         */
        private var _response: String
        private var _errorResponse: String

        /**
            Create an Cmd object. If a command line is provided, the command is immediately started.
            @param command The (optional) command line to initialize with. If a command line is provided, the start()
                method is automatically invoked after the command is constructed. The command may be either a string or
                an array of arguments. Using an array of args can simplify quoting if the args have embedded spaces or
                quotes.
            @param options. Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to false.
            @options timeout Number This is the default number of milliseconds for the command to complete.
         */
        native function Cmd(command: Object = null, options: Object = null)

        /**
            Close the connection to the command and free up all associated resources. It is not normally required to call 
            $close, but it can be useful to force a command termination. After calling close, the command status and
            unread response data are not accessible.
         */
        native function close(): Void 

        /**
            Hash of environment strings to pass to the command.
         */
        native function get env(): Object
        native function set env(values: Object): Void

        /**
            Eommand error output data as a string. The first time this property is read, the error content will be read 
            and buffered.
         */
        function get error(): String {
            if (!_errorResponse) {
                _errorResponse = errorStream.toString()
            }
            return _errorResponse
        }

        /**
            The error stream object for the command's standard error output 
         */
        native function get errorStream(): Stream

        /** 
            Signal the end of write data. The finalize() call must be invoked to properly signify the end of write data.
         */
        native function finalize(): Void 

        /**
            This call has no effect
            @duplicate Stream.flush
            @hide
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /** 
            @duplicate Stream.on
            @event readable Issued when output data to read.
            @event writable Issued when the command can accept more write data.
            @event complete Issued when the command completes
            @event error Issued if the $error stream is readable.

            All events are called with the signature:
            function (event: String, cmd: Cmd): Void
         */
        native function on(name, listener: Function): Void

        /** 
            This call is not supported.
            @duplicate Stream.off
            @hide
         */
        native function off(name, listener: Function): Void

        /**
            Process ID of the the command. This is set to the process ID (PID) of the command. If no command has 
            started or the command has called $close, the call with throw an exception.
         */
        native function get pid(): Number

        /**
            @duplicate Stream.read
            If no listener has been defined via $on(), this call will block if there is no data to be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /**
            Read the data from the command output as a string. This reads from the command's standard output. 
            @param count of bytes to read. Returns the entire output data contents if count is -1.
            @returns a string of $count characters beginning at the start of the output data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String

        /**
            Read the data from the command as an array of lines. This reads from the command's standard output.
            @param count of linese to read. Returns the entire output contents if count is -1.
            @returns a string containing count lines of data starting with the first line of output data
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /**
            Read the command output as an XML document. This reads from the command's standard output.
            @returns the output content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML
            XML(readString())

        /**
            Command output data as a string. This is an alias for $readString() but it will cache the 
                output data and may be called multiple times. This reads from the command's standard output.
         */
        function get response(): String {
            if (!_response) {
                _response = readString()
            }
            return _response
        }

        /**
            Start the command.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @param options Command options. Sames as options in $Cmd
            @throws IOError if the request was cannot be issued to the remote server.
         */
        native function start(cmdline: Object, options: Object = null): Void

        /**
            Get the command exit status. This command will block until the command has completed. Use wait(0) to 
            test if the command has completed.
            @return The command exit status
         */
        native function get status(): Number

        /**
            Stop the current command. After stopping, the command exit status and any response or error data are
            available for interrogation.
            @param signal Signal to send the the active process.
            @return True if successful
          */
        native function stop(signal: Number = 2): Boolean

        /**
            Default command timeout for wait(), read(), and blocking start(). If the timeout expires, the command is
            not killed. Rather any blocking calls will simply return. 
            This is the number of milliseconds for the call to complete.
         */
        native function get timeout(): Number
        native function set timeout(msec: Number): Void

        /**
            Wait for a command to complete. 
            @param timeout Time in seconds to wait for the command to complete. If unspecified, the $timeout propoperty
                value is used instead.
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /**
            @duplicate Stream.write
            Call finalize() to signify the end of write data. Failure to call finalize() may prevent some commands 
            from exiting.
         */
        native function write(...data): Number


        /* Static Helper Methods */

        /**
            Start a command in the background as a daemon.  No data can be written to the daemon's stdin.
            @return The process ID. This pid can be used with kill().
         */
        static function daemon(cmdline: Object, options: Object = null): Number {
            let cmd = new Cmd
            options ||= {}
            blend(options, {detach: true})
            cmd.start(cmdline, options)
            cmd.start()
            cmd.finalize()
            return cmd.pid
        }

        /**
            Execute a new program by the current process. This Replaces the current program with another without
            creating a new process.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @param options Command options. Sames as options in $Cmd
            @throws IOError if the request was cannot be issued to the remote server.
         */
        native static function exec(cmdline: String = null, options: Object = {}): Void

        /**
            @param signal If pid is greater than zero, the signal is sent to the process whoes ID is pid. If pid is
                zero, the signal is sent to all processes in the process group.
            @return True if successful
         */
        native static function kill(pid: Number, signal: Number = 2): Boolean

        /**
            Execute a command/program. The call blocks while executing the command.
            @param command Command or program to execute
            @param data Optional data to write to the command on it's standard input.
            @returns The command output from it's standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function run(command: Object, data: Object = null): String {
            let cmd = new Cmd
            cmd.start(command, {detach: true})
            if (data) {
                cmd.write(data)
            }
            cmd.finalize()
            cmd.wait()
            if (cmd.status != 0) {
                throw new IOError(cmd.error)
            }
            return cmd.readString()
        }

        /**
            Run a command using the system command shell and wait for completion. On Windows, this requires that
            /bin/sh.exe is installed (See Cygwin). 
            @param command The (optional) command line to initialize with. The command may be either a string or
                an array of arguments. 
         */
        static function sh(command: Object, data: Object = null): String {
            /*
                The form is:  /bin/sh -c "command args"
                The args must be wrapped in single quotes if they contain spaces. 
                Example:
                    This:       ["showColors", "red", "light blue", "Can't \"render\""]
                    Becomes:    /bin/sh -c "showColors red 'light blue' 'Can\'t \"render\"'
             */
            if (command is Array) {
                for (let arg in command) {
                    /*  
                        Backquote backslashes and backquote quotes. Then wrap in single quotes. Single quotes are 
                        required because Cmd on Windows must format the entire command as a single string (not argv[])
                     */
                    let s = command[arg].toString().trimEnd('\n')           // .replace(/\\/g, "\\\\")
                    s = s.replace(/\"/g, '\\\"').replace(/\'/g, '\\\'')
                    command[arg] = "'" + s + "'"
                }
                return run(["/bin/sh", "-c"] + [command.join(" ")], data).trimEnd()
            }
            /*
                Must quote single and double quotes as the comand will be wrapped in quotes on Windows.
                WARNING: If starting a program compiled with Cygwin, Cygwin has a bug where embedded quotes are parsed
                incorrectly by the Cygwin crt runtime startup. If an arg starts with a drive spec, embedded backquoted 
                quotes will be stripped and the backquote will be passed in. Windows crt runtime handles this correctly.
                For example:  ./args "c:/path \"a b\"
                    Cygwin will parse as  argv[1] == c:/path \a \b
                    Windows will parse as argv[1] == c:/path "a b"
             */
            command = command.toString().trimEnd('\n').replace(/\"/g, '\\\"')   // .replace(/\'/g, '\\\'')
            return run(["/bin/sh", "-c", command], data).trimEnd()
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
    
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
