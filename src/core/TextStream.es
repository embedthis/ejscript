/*
    TextStream.es -- TextStream class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        TextStreams interpret data as a stream of characters. They provide methods to read and write data
        in various text encodings and to read/write lines of text appending appropriate system dependent new line 
        terminators. TextStreams can be stacked upon other Streams such as files, byte arrays, sockets, or Http objects.
        @spec ejs
        @stability evolving
     */
    class TextStream implements Stream {

        use default namespace public

        /** 
            System dependent newline terminator
         */
        private var newline: String

        /* 
            Data input buffer
         */
        private var inbuf: ByteArray

        /** @hide */
        private var format: String = "utf-8"

        /* 
            Provider Stream
         */
        private var nextStream: Stream

        /** 
            Create a text filter stream. A Text filter stream must be stacked upon a stream source such as a File.
            @param stream stream data source/sink to stack upon.
         */
        function TextStream(stream: Stream?) {
            if (stream == null) {
                throw new ArgError("Must supply a Stream argument")
            }
            inbuf = new ByteArray
            nextStream = stream
            newline = FileSystem("/").newline
        }

        /** 
            @duplicate Stream.async 
         */
        function get async(): Boolean
            false

        /** 
            @duplicate Stream.async 
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("Async mode not supported")
            }
        }

        /** 
            @duplicate Stream.close
         */
        function close(): Void {
            inbuf.flush(2 /* MOB Stream.WRITE */)
            nextStream.close()
        }

        /** 
            The current text encoding.
            Not implemented.
            @hide
         */
        function get encoding(): String
            format

        /** 
            Set the current text encoding. Currently not supported or used.
            Not implemented.
            @param encoding string containing the current text encoding. Supported encodings are: utf-8.
            @hide
         */
        function set encoding(encoding: String): Void {
            format = encoding
        }

        /**  
            Fill the input buffer from upstream
            @returns The number of new characters added to the input bufer
         */
        function fill(): Number? {
            inbuf.compact()
            return nextStream.read(inbuf, -1)
        }

        /** 
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.WRITE)
                inbuf.flush()
            if (!(nextStream is ByteArray)) {
                nextStream.flush(dir)
            }
        }

        /** 
            The number of bytes available to read without blocking. This is the number of bytes buffered internally
            by this stream. It does not include any data buffered downstream.
            @return the number of available bytes
         */
        function get length(): Number
            inbuf.length

        /** 
            @duplicate Stream.off
         */
        function off(name, observer: Function): Void {
            throw new ArgError("Observers are not supported")
        }

        /** 
            @duplicate Stream.on 
         */
        function on(name, observer: Function): TextStream {
            throw new ArgError("Observers are not supported")
            return this
        }

        /** 
            Read characters from the stream into the supplied byte array. 
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of characters to read. 
            @returns a count of characters actually read
            @throws IOError if an I/O error occurs.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? {
            let total = 0
            if (count < 0) {
                count = Number.MaxValue
            }
            if (offset < 0) {
                buffer.reset()
            } else {
                buffer.flush(1 /* MOB Stream.READ */)
            }
            let where = buffer.writePosition
            while (count > 0) {
                if (inbuf.length == 0) {
                    if (fill() <= 0) {
                        if (total == 0) {
                            return null
                        }
                        break
                    }
                }
                let len = count.min(inbuf.length)
                len = len.min(buffer.size - where)
                if (len == 0) break
                len = buffer.copyIn(where, inbuf, inbuf.readPosition, len)
                inbuf.readPosition += len
                where += len
                total += len
                count -= len
            }
            buffer.writePosition += total
            return total
        }

        /** 
            Read a line from the stream.
            @returns A string containing the next line without newline characters ("\r", "\n"). Return null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readLine(): String? {
            if (inbuf.length == 0 && fill() <= 0) {
                return null
            }
            //  All systems strip both \n and \r\n to normalize text lines
            //  MOB -- this should be a configurable option on a TextStream
            let nl = "\r\n"
            let nlchar = nl.charCodeAt(nl.length - 1)
            let nlchar0 = nl.charCodeAt(0)
            while (true) {
                for (let i = inbuf.readPosition; i < inbuf.writePosition; i++) {
                    //  MOB OPT. If ByteArray had indexOf(nl), then this could be MUCH faster
                    if (inbuf[i] == nlchar) {
                        if (nl.length == 2 && i > inbuf.readPosition && nlchar0 == inbuf[i-1]) {
                            result = inbuf.readString(i - inbuf.readPosition - 1)
                            inbuf.readPosition += 2
                        } else {
                            result = inbuf.readString(i - inbuf.readPosition)
                            inbuf.readPosition++
                        }
                        return result
                    }
                }
                if (fill() <= 0) {
                    /* Missing a line terminator, so return any last portion of text */
                    if (inbuf.length) {
                        return inbuf.readString(inbuf.length)
                    }
                }
            }
            return null
        }

        /** 
            Read a required number of lines of data from the stream.
            @param numLines of lines to read. Defaults to read all lines.
            @returns Array containing the read lines. Lines are stripped of newline characters ("\r", "\n"). 
            Return null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readLines(numLines: Number = -1): Array? {
            var result: Array
            if (numLines <= 0) {
                result = new Array
                numLines = Number.MaxValue
            } else {
                result = new Array(numLines)
            }
            for (let i in numLines) {
                if ((line = readLine()) == null) {
                    if (i == 0) {
                        return null
                    }
                    break
                }
                result[i] = line
            }
            return result
        }

        /** 
            Read a string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string or null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readString(count: Number = -1): String?
            inbuf.readString(count)

        /** 
            Write characters to the stream.
            @param data String to write. 
            @returns The total number of bytes that were written.
         */
        function write(...data): Number
            nextStream.write(data)

        /** 
            Write text lines to the stream. The text line is written after appending the system text newline character(s).
            @param lines Text lines to write.
            @return The number of characters written or -1 if unsuccessful.
         */
        function writeLine(...lines): Number {
            let written = 0
            if (lines.length == 0) {
                nextStream.write("\n");
                written++
            } else {
                for each (let line in lines) {
                    nextStream.write(line)
                    /* If the file is opened in text mode, the lower layers will add "\r" for windows */
                    nextStream.write("\n");
                    written += line.length + 1
                }
            }
            return written
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.md distributed with 
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
