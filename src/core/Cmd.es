/*
    Cmd.es - Cmd class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Cmd class supports invoking other programs as separate processes on the same system. 
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
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
         */
        native function Cmd(command: Object = null, options: Object = null)

        /**
            Close the connection to the command and free up all associated resources. It is not normally required to call 
            $close, but it can be useful to force a command termination. After calling close, the command status and
            unread response data are not accessible.
         */
        native function close(): Void 

        //  TODO - should be a function. Need flags MPR_CMD_EXACT_ENV support.
        /**
            Hash of environment strings to pass to the command.
         */
        native function get env(): Object
        native function set env(values: Object): Void

        /**
            Command error output data as a string. The first time this property is read, the error content will be read 
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
            Signal the end of writing data to the command. The finalize() call must be invoked to properly 
            signify the end of write data.
         */
        native function finalize(): Void 

        /**
            This call has no effect
            @duplicate Stream.flush
            @hide
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /**
            Locate a program using the application PATH
            @param program Program to find
            @param search Optional additional search paths to use before using PATH
            @return Located path or null
         */
        static function locate(program: Path, search = []): Path? {
            search += App.getenv("PATH").split(App.SearchSeparator)
            for each (dir in App.getenv("PATH").split(App.SearchSeparator)) {
                let path = Path(dir).join(program)
                if (path.exists && !path.isDir) {
                    return path
                }
            }
            if (Config.OS == 'windows' || Config.OS == 'cygwin') {
                if (program.extension == '') {
                    for each (ext in ['exe', 'bat', 'cmd']) {
                        let path = locate(program.joinExt('.exe'))
                        if (path) {
                            return path;
                        }
                    }
                }
            }
            return null
        }

        /** 
            @duplicate Stream.on
            @event readable Issued when output data to read.
            @event writable Issued when the command can accept more write data.
            @event complete Issued when the command completes
            @event error Issued if the $error stream is readable.

            All events are called with the signature:
            function (event: String, cmd: Cmd): Void
         */
        native function on(name, observer: Function): Cmd

        /** 
            This call is not supported.
            @duplicate Stream.off
            @hide
         */
        native function off(name, observer: Function): Void

        /**
            Process ID of the the command. This is set to the process ID (PID) of the command. If no command has 
            started or the command has called $close, the call with throw an exception.
         */
        native function get pid(): Number

        /**
            @duplicate Stream.read
            If no observer has been defined via $on(), this call will block if there is no data to be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /**
            Read the data from the command output as a string. This reads from the command's standard output. 
            @param count of bytes to read. Returns the entire output data contents if count is -1.
            @returns a string of $count characters beginning at the start of the output data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String?

        /**
            Read the data from the command as an array of lines. This reads from the command's standard output.
            @param count of linese to read. Returns the entire output contents if count is -1.
            @returns a string containing count lines of data starting with the first line of output data
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array? {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /**
            Read the command output as an XML document. This reads from the command's standard output.
            @returns the output content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML?
            XML(readString())

        /**
            Command output data as a string. This is an alias for $readString() but it will cache the 
                output data and may be called multiple times. This reads from the command's standard output.
         */
        function get response(): String? {
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
            @return True if the command is successfully stopped or the command has completed.
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
            @param timeout Time in milliseconds to wait for the command to complete. If unspecified, the $timeout propoperty
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
            Start a command in the background as a daemon.  The daemon command is detached and the application 
            continues immediately in the foreground. Note: No data can be written to the daemon's stdin.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @return The process ID. This PID can be used with kill().
         */
        static function daemon(cmdline: Object, options: Object = null): Number {
            let cmd = new Cmd
            options ||= {}
            cmd.start(cmdline, blend({detach: true}, options))
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
        native static function exec(cmdline: String? = null, options: Object = {}): Void

        /**
            Kill the specified process.
            @param pid Process ID of the process to kill
            @param signal If pid is greater than zero, the signal is sent to the process whoes ID is pid. If pid is
                zero, the process is tested but no signal is sent. 
            @return True if successful
            @throws IOError if the pid is invalid or if the requesting process does not have sufficient privilege to
                send the signal.
         */
        native static function kill(pid: Number, signal: Number = 2): Boolean

        /** 
            Kill all matching processes. This call enables selection of the processes to kill a pattern match over
            the processes command line. Note: this command does not throw exceptions if a matching process cannot be
            killed. Use kill() for reliable process execution.
            @param pattern of processes to kill. This can be a string name or a regular expression to match with.
            @param signal Signal number to send to the processes to kill. If the signal is not supplied, then the system
            default signal is sent (SIGTERM).
            @param preserve Optional set of process IDs to preserve
            @hide 
         */
        static function killall(pattern: Object, signal: Number = 15, ...preserve): Void {
            let cmd = new Cmd
            if (Config.OS == "windows" || Config.OS == "cygwin") {
                cmd.start('cmd /A /C "WMIC PROCESS get Processid,Commandline /format:csv"')
                for each (line in cmd.readLines()) {
                    let fields = line.trim().split(",")
                    let pid = fields.pop().trim()
                    let command = fields.slice(1).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        if (preserve.length == 0 || !preserve.find(function(e, index, arrr) { return e == pid })) {
                            // print("KILL " + pid + " pattern " + pattern + " signal " + signal)
                            try {
                                Cmd.kill(pid, signal)
                            } catch {}
                        }
                    }
                }
            } else {
               cmd.start("/bin/ps -e")
                for each (line in cmd.readLines()) {
                    let fields = line.split(/ +/g)
                    let pid = fields[0]
                    let command = fields.slice(3).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        if (preserve.length == 0 || !preserve.find(function(e, index, arrr) { return e == pid })) {
                            // print("KILL " + pid + " pattern " + pattern + " signal " + signal)
                            try {
                                Cmd.kill(pid, signal)
                            } catch {}
                        }
                    }
                }
            }
            cmd.close()
        }

        /** 
            Return a list of processes running on the system. This enables selection of processes by string pattern or by
            a regular expression against the entire process command line.
            @param pattern Pattern to use when selecting matching processes. This can be a string name or a regular 
            expression to match with. A string name will match if the processes's command line contains the pattern as
            a sub-string.
            @return An array of matching processes. Each array entry is an object with properties "pid" and "command".
            @hide 
         */
        static function ps(pattern: Object = ""): Array {
            let result = []
            let cmd = new Cmd
            if (Config.OS == "windows" || Config.OS == "cygwin") {
                cmd.start('cmd /A /C "WMIC PROCESS get Processid,Commandline /format:csv"')
                for each (line in cmd.readLines()) {
                    let fields = line.split(",")
                    let pid = fields.pop().trim()
                    let command = fields.slice(1).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        result.append({pid: pid, command: command})
                    }
                }
                //  Windows WMIC drops this
                Path("TempWmicBatchFile.bat").remove()
            } else {
                cmd.start("/bin/ps -ef")
                for each (line in cmd.readLines()) {
                    let fields = line.trim().split(/ +/g)
                    let pid = fields[1]
                    let command = fields.slice(7).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        result.append({pid: pid, command: command})
                    }
                }
            }
            cmd.close()
            return result
        }

        /**
            Execute a command/program and return the output as a result. 
            The call blocks while executing the command.
            @param command Command or program to execute
            @param options Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to true.
            @options timeout Number This is the default number of milliseconds for the command to complete.
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
            @param data Optional data to write to the command on it's standard input.
            @returns The command output from the standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function run(command: Object, options: Object = {}, data: Object = null): String {
            //  TODO - the above default arg should handle this
            options ||= {}
            let cmd = new Cmd
            cmd.start(command, blend({detach: true}, options))
            if (data) {
                cmd.write(data)
            }
            cmd.finalize()
            cmd.wait()
            if (cmd.status != 0 && !options.exception) {
                throw new IOError(cmd.error)
            }
            return cmd.readString()
        }

        /**
            Run a command using the system command shell and wait for completion. On Windows, this requires that
            sh.exe is installed (See Cygwin). 
            @param command The (optional) command line to initialize with. The command may be either a string or
                an array of arguments. 
            @param options Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to false.
            @options timeout Number This is the default number of milliseconds for the command to complete.
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
            @param data Optional data to write to the command on it's standard input.
            @return The command output from the standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function sh(command: Object, options: Object = null, data: Object = null): String {
            /*
                The form is:  sh -c "command args"
                The args must be wrapped in single quotes if they contain spaces. 
                Example:
                    This:       ["showColors", "red", "light blue", "Cannot \"render\""]
                    Becomes:    sh -c "showColors red 'light blue' 'Cannot \"render\"'
             */
            let shell = Cmd.locate("sh")
            if (command is Array) {
                for (let arg in command) {
                    /*  
                        Backquote backslashes and backquote quotes. Then wrap in single quotes. Single quotes are 
                        required because Cmd on Windows must format the entire command as a single string (not argv[])
                     */
                    let s = command[arg].toString().trimEnd('\n')
                    s = s.replace(/\"/g, '\\\"').replace(/\'/g, '\\\'')
                    command[arg] = "'" + s + "'"
                }
                return run([shell, "-c"] + [command.join(" ")], options, data).trimEnd()
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
            return run([shell, "-c", command.toString().trimEnd('\n')], options, data).trimEnd()
        }
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
