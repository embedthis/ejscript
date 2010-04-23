/*
    BinaryStream.es -- BinaryStream class. This class is a filter or endpoint stream to encode and decode binary types.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        BinaryStreams encode and decode various objects onto streams. A BinaryStream may be stacked atop an underlying stream
        provider such as ByteArray, File, Http or Socket. The underlying stream must be in sync mode.
        @spec ejs
        @stability evolving
     */
    class BinaryStream implements Stream {

        use default namespace public

        /** 
            Big endian byte order 
         */
        static const BigEndian: Number = ByteArray.BigEndian

        /** 
            Little endian byte order 
         */
        static const LittleEndian: Number = ByteArray.LittleEndian

        /* 
            Data input and output buffers. The buffers are used to marshall the data for encoding and decoding. The inbuf 
            also  hold excess input data. The outbuf is only used to encode data -- no buffering occurs.
         */
        private var inbuf: ByteArray
        private var outbuf: ByteArray
        private var nextStream: Stream

        /** 
            Create a new BinaryStream
            @param stream stream to stack upon.
         */
        function BinaryStream(stream: Stream) {
            if (!stream) {
                throw new ArgError("Must supply a Stream to connect with")
            }
            nextStream = stream
            inbuf = new ByteArray
            outbuf = new ByteArray

            inbuf.addListener("writable", function (event: String, buffer: ByteArray) {
                nextStream.read(buffer)
            })

            outbuf.addListener("readable", function (event: String, buffer: ByteArray) {
                count = nextStream.write(buffer)
                buffer.readPosition += count
                buffer.reset()
            })
        }

        /** 
            @duplicate Stream.addListener 
         */
        native function addListener(name, listener: Function): Void

        /** 
            @duplicate Stream.async 
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async 
         */
        native function set async(enable: Boolean): Void

        /** 
            The number of bytes available to read
            @return the number of available bytes
         */
        function get available(): Number
            inbuf.available

        /** 
            @duplicate Stream.close 
         */
        function close(): void
            nextStream.close()

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String {
            return inbuf.enc
        }

        /** 
            @duplicate BinaryStream.encoding
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            inbuf.encoding = enc
            outbuf.encoding = enc
        }

        /** 
            Current byte ordering. Set to either LittleEndian or BigEndian.
         */
        function get endian(): Number
            inbuf.endian

        /** 
            Set the system encoding to little or big endian.
            @param value Set to true for little endian encoding or false for big endian.
         */
        function set endian(value: Number): Void {
            if (value != BigEndian && value != LittleEndian) {
                throw new ArgError("Bad endian value")
            }
            inbuf.endian = value
            outbuf.endian = value
        }

        /**
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.READ) 
                inbuf.flush(Stream.READ)
            if (dir & Stream.WRITE) 
                outbuf.flush(Stream.WRITE)
            if (!(nextStream is ByteArray)) {
                /* Don't flush loopback bytearrays */
                nextStream.flush(dir)
            }
        }

        /** 
            @duplicate Stream.read
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
            inbuf.read(buffer, offset, count)

        /** 
            Read a boolean from the stream.
            @returns a boolean. Returns null on eof.
            @throws IOError if an I/O error occurs.
         */
        function readBoolean(): Boolean
            inbuf.readBoolean()

        /** 
            Read a byte from the stream.
            @returns a byte. Returns -1 on eof.
            @throws IOError if an I/O error occurs.
         */
        function readByte(): Number
            inbuf.readByte()

        /** 
            MOB -- unwanted
            Read data from the stream into a byte array.
            @returns a new byte array with the available data. Returns an empty byte array on eof.
            @throws IOError if an I/O error occurs or premature eof
        function readByteArray(count: Number = -1): ByteArray
            inbuf.readByteArray(count)
         */

        /** 
            Read a date from the stream.
            @returns a date
            @throws IOError if an I/O error occurs or premature eof
         */
        function readDate(): Date
            inbuf.readDate()

        /** 
            Read a double from the stream. The data will be decoded according to the encoding property.
            @returns a double
            @throws IOError if an I/O error occurs or premature eof
         */
        function readDouble(): Double
            inbuf.readDouble()

        /** 
            Read a 32-bit integer from the stream. The data will be decoded according to the encoding property.
            @returns an 32-bitinteger
            @throws IOError if an I/O error occurs or premature eof
         */
        function readInteger(): Number
            inbuf.readInteger()

        /** 
            Read a 64-bit long from the stream.The data will be decoded according to the encoding property.
            @returns a 64-bit long number
            @throws IOError if an I/O error occurs or premature eof
         */
        function readLong(): Number
            inbuf.readInteger()

        /** 
            Read a UTF-8 string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string
            @throws IOError if an I/O error occurs or premature eof.
         */
        function readString(count: Number = -1): String 
            inbuf.readString(count)

        /** 
            Read an XML document from the stream. This assumes the XML document will be the only data until eof.
            @returns an XML document
            @throws IOError if an I/O error occurs or premature eof
         */
        function readXML(): XML {
            var data: String = ""
            while (1) {
                var s: String = inbuf.readString()
                if (s == null && data.length == 0) {
                    return null
                }
                if (s.length == 0) {
                    break
                }
                data += s
            }
            return new XML(data)
        }

        /** 
            @duplicate Stream.addListener 
         */
        native function removeListener(name, listener: Function): Void

        /** 
            Return the space available for write data. This call can be used to prevent write from blocking or 
            doing partial writes. If it cannot be determined how much room is available, this call will return null.
            @return The number of bytes that can be written without blocking or null if it cannot be determined.
         */
        function room(): Number
            outbuf.room()
       
        /** 
            Write data to the stream. Write intelligently encodes various data types onto the stream and will encode 
            data in a portable cross-platform manner according to the setting of the $endian property. If data is an 
            array, each element of the array will be written. The write call blocks until the underlying stream or 
            endpoint absorbes all the data. 
            @param items Data items to write. The ByteStream class intelligently encodes various data types according 
            to the current setting of the $endian property. 
            @returns The total number of elements that were written.
            @throws IOError if there is an I/O error.
            @event 
         */
        function write(...items): Number {
            let count: Number = 0
            for each (i in items) {
                count += outbuf.write(i)
            }
            return count
        }

        /** 
            Write a byte to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeByte(data: Number): Void 
            outbuf.writeByte(outbuf)

        /** 
            Write a short to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeShort(data: Number): Void
            outbuf.writeShort(data)

        /** 
            Write a double to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeDouble(data: Number): Void
            outbuf.writeDouble(data)

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeInteger(data: Number): Void
            outbuf.writeInteger(data)

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeLong(data: Number): Void
            outbuf.writeLong(data)
    }
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
