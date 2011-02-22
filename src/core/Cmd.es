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

        /**
            Create an Cmd object. The Cmd object is initialized with the optional command line. If a command line is
            provided, the command is immediately started.
            @param cmdline The (optional) command line to initialize with. If a command line is provided, the start()
                method is automatically invoked after the command is constructed. The cmdline may be either a string or
                an array of arguments.
            @param options. Command options hash. Supported options are:
            @options detach Boolean If true, run the command in the background. Do not capture the command's stdout or
                stderr, nor collect status. Defaults to false.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to false.
         */
        native function Cmd(cmdline: Object = null, options: Object = null)

        /**
            Close the connection to the command and free up all associated resources. It is not normally required to call 
            $close. But it can be useful to force a command termination. After calling close, the command status and
            unread response data are not accessible.
MOB - TEST
         */
        native function close(): Void 

        /**
            The error stream object for the command's stderr output 
MOB - TEST
         */
        native function get error(): Stream

        /**
            Hash of environment strings for the command.
MOB - TEST
         */
        native function get env(): Object
        native function set env(values: Object): Void

        /** 
            Signals the end of write data. Finalize() must be called to properly signify the end of write data.
MOB - TEST
         */
        native function finalize(): Void 

        /**
            @duplicate Stream.flush
MOB - TEST
         */
//  MOB - what does this do?
        native function flush(dir: Number = Stream.BOTH): Void

        /** 
            @duplicate Stream.on
            @event readable Issued when output data to read.
            @event writable Issued when the connection to the command is writable to accept data.
            @event complete Issued when the command completes
            @event error Issued if the $error stream is readable.

            All events are called with the signature:
            function (event: String, cmd: Cmd): Void
MOB - why have cmd as an arg. Surely "this" will be set to the cmd?
MOB - TEST
         */
        native function on(name, listener: Function): Void

        /** 
            @duplicate Stream.off
MOB - TEST
         */
        native function off(name, listener: Function): Void

        //  MOB - return 0 if command not started or killed
        native function get pid(): Number

        /**
            @duplicate Stream.read
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
            Return the command output data as a string. This is an alias for $readString() but it will cache the 
                output data and may be called multiple times. This reads from the command's standard output.
            @returns the output as a string of characters.
            @throws IOError if an I/O error occurs.
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
// MOB - confusing allowing Cmd(command) and Cmd().start(command)
        native function start(cmdline: Object, options: Object = {}): Void

        /**
            Get the command exit status. This command will block until the command has completed. Use wait(0) to 
            test if the command has completed.
            @return The command exit status
         */
        native function get status(): Number

        native function stop(signal: Number = 2): Boolean

        /**
            Command timeout before the command is forcibly killed and a completion event issued. 
            This is the number of milliseconds for the command to complete.
MOB - TEST
         */
        native function get timeout(): Number
        native function set timeout(msec: Number): Void

        /**
            Wait for a command to complete.
            @param timeout Time in seconds to wait for the command to complete. If unspecified the $timeout propoperty
                value is used instead
            @return True if the request successfully completes.
MOB - TEST
         */
        native function wait(timeout: Number = -1): Boolean

        /**
            @duplicate Stream.write
            Call finalize to signify the end of write data.
         */
        native function write(...data): Number


        /* Static Helper Methods */

        static function daemon(cmdline: Object): Number {
            let cmd = new Cmd
            cmd.start(cmdline, {detach: true})
            cmd.start()
            return cmd.pid
        }

        //  MOB - similar to start, but replaces the current process
        native function exec(cmdline: String = null, options: Object = {}): Void

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
        static function run(cmdline: Object, data: Object = null): String {
            let cmd = new Cmd
            cmd.start(cmdline)
            if (data) {
                cmd.write(data)
                cmd.finalize()
            }
            cmd.wait()
            if (cmd.status != 0) {
                //  MOB - error does not have readString()
print("STATUS " + cmd.status)
print("TYPE " + typeOf(cmd.error))
                throw new IOError(cmd.error.readString())
            }
            return cmd.readString()
        }

        /*
            Run a command using the system command shell and wait for completion. This supports pipelines.
            Any response trailing newline is trimmed.
MOB - describe throw
         */
        static function sh(command: Object, data: Object = null): String {
            if (command is String) {
                //  MOB - validate quoting and look on forum
                return run(("/bin/sh -c \"" + command.replace(/\\/g, "\\\\") + "\"").trim('\n'), data).trimEnd()
            } else {
                let args = command.join(" ").replace(/\\/g, "\\\\").trim('\n')
                return run(["/bin/sh", "-c"] + [args], data).trimEnd()
            }
        }
    }

    //  DOC
    //  MOB - or should this be named sh()
    function sh(cmdline: Object, data: Object = null): String
        Cmd.sh(cmdline, data)
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
