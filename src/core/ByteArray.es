/*
    ByteArray.es - ByteArray class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    use strict

    /** 
        ByteArrays provide a growable, integer indexed, in-memory store for bytes. ByteArrays can be used as a simple 
        array type to store and encode data as bytes or they can be used as buffered loop-back Streams.
        
        When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
        extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
        can be used to get and put blocks of data. In this mode, the $readPosition and $writePosition properties are ignored.

        Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
        the ByteArray can be designated as growable, in which case the initial size will grow as required to accomodate 
        data and the length property will be updated accordingly.
        
        ByteArrays provide additional write methods to store data at the location specified by the $writePosition 
        property and read methods to read from the $readPosition property. The $available property indicates how much 
        data is available between the read and write position pointers. The $reset method can reset the pointers to 
        the start of the array.  When used with for/in, ByteArrays will iterate or enumerate over the available 
        data between the read and write pointers.

        If numeric values are read or written, they will be coded according to the value of the endian property 
        which can be set to either LittleEndian or BigEndian.  If strings values are read or written, they will 
        be encoded according to the value of the character set $encoding property.
        
        When used as loop-back streams, data written to ByteArrays is immediately available for reading. 
        ByteArrays can be run in sync or async mode. ByteArrays will issue events for key state transitions such as 
        close, eof, readable and writable events. All event observers are called with the following signature:
            function callback(event: String, ba: ByteArray): Void
        @spec ejs
        @stability evolving
     */
    final class ByteArray implements Stream {

        use default namespace public

        /** 
            Little endian byte order used for the $endian property
         */
        static const LittleEndian: Number   = 0

        /** 
            Big endian byte order used for the $endian property
         */
        static const BigEndian: Number      = 1

        /** 
            Create a new array.
            @param size The initial size of the byte array. If not supplied a default buffer size will be used which is
            typically 4K or larger.
            @param growable Set to true to automatically grow the array as required to fit written data.
         */
        native function ByteArray(size: Number = -1, growable: Boolean = true)

        /** 
            Number of bytes that are currently available for reading. This consists of the bytes available
            from the current $readPosition up to the current $writePosition.
         */
        native function get available(): Number 

        /** @duplicate Stream.async */
        native function get async(): Boolean

        /** @duplicate Stream.async */
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        /** 
            Compact available data down and adjust the read/write positions accordingly. This sets the read pointer 
            to the zero index and adjusts the write pointer by the corresponding amount.
         */
        native function compact(): Void

        /** 
            Compress the array contents.
         */
        # FUTURE
        native function compress(): Void

        /** 
            Copy data into the array. This is a low-level data copy routine that does not update read and write positions.
            Data is written at the destOffset index. This call does not issue events unless required to make room
            for the incoming data ("readable" event).
            @param destOffset Index in the destination byte array to copy the data to
            @param src Source byte array containing the data elements to copy
            @param srcOffset Location in the source buffer from which to copy the data. Defaults to the start.
            @param count Number of bytes to copy. Set to -1 to read all the src buffer.
            @return the number of bytes written into the array. If the array is not growable and there is insufficient
                room, this may be less than the requested amount.
         */
        native function copyIn(destOffset: Number, src: ByteArray, srcOffset: Number = 0, count: Number = -1): Number

        /** 
            Copy data from the array. Data is copied from the srcOffset pointer. This call does not update the 
                read and write positions. This call does not issue events.
            @param srcOffset Location in the source array from which to copy data.
            @param dest Destination byte array
            @param destOffset Location in the destination array to copy the data. Defaults to the start.
            @param count Number of bytes to read. Set to -1 to read all available data.
            @returns the count of bytes read. Returns null on eof.
         */
        native function copyOut(srcOffset: Number, dest: ByteArray, destOffset: Number = 0, count: Number = -1): Number

        /** 
            Current encoding scheme for serializing strings. The default encoding is "utf-8". Set to "" for no encoding.
            If no encoding, string character points are stored as a pairs of two byte in little-endian format.
         */
        function get encoding(): String 
            "utf-8"

        /** 
            Current encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
         */
        native function get endian(): Number

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
            @param value Set to true for little endian ordering or false for big endian.
         */
        native function set endian(value: Number): Void

        /** 
            Flush (discard) the data in the byte array and reset the read and write positions. 
            This call may block if the stream is in sync mode.
            @param dir The dir parameter is Ignored. Flushing a ByteArray in either direction the same effect of 
                discarding all buffered data and resetting the read and write positions -- so this argument is ignored. 
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /**
            Is the ByteArray is growable.
         */
        native function get growable(): Boolean

        /** 
            Iterator for this array to be used by "for (v in array)". This will return array indicies.
         */
        override iterator native function get(): Iterator

        /** 
            Iterator for this array to be used by "for each (v in array)". This will return read data in the array.
         */
        override iterator native function getValues(): Iterator

        /** 
            Length of the byte array. This is not the amount of read or write data, but is the size of the total 
            array storage.
         */
        native function get length(): Number

        /**
            An MD5 checksum for the buffer contents
         */
        function get MD5(): String {
            let buf: ByteArray = this.clone()
            return md5(buf.readString())
        }

        /** @duplicate Stream.off */
        native function off(name: Object, observer: Function): Void

        /** @duplicate Stream.on */
        native function on(name: Object, observer: Function): Void

        /** 
            @duplicate Stream.read
            Data is read from the current read $position pointer toward the current $writePosition. 
            This byte array's $readPosition is updated. If offset is < 0, then data is copied to the destination buffer's 
            $writePosition and the destination buffer's $writePosition is also updated. If the offset is >= 0, the 
            read position is set to the specified offset and data is stored at this offset. The write position is set to
            one past the last byte read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read a boolean from the array. Data is read from the current read $position pointer.
            If insufficient data, a "writable" event will be issued indicating that the byte array is writable. This enables 
            observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @returns a boolean or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readBoolean(): Boolean

        /** 
            Read a byte from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readByte(): Number

        /** 
            Read a date from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readDate(): Date

        /** 
            Read a double from the array. The data will be decoded according to the endian property. Data is read 
            from the current read $position pointer. If insufficient data, a "write" event will be issued indicating 
            that the byte array is writable. This enables observers to write data into the byte array. If there is 
            no data available, the call will return return null indicating eof.
            @returns a double or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readDouble(): Date

        /** 
            Read an 32-bit integer from the array. The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readInteger(): Number

        /** 
            Read a 64-bit long from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readLong(): Number

        /** 
            Current read position offset
         */
        native function get readPosition(): Number

        /** 
            Set the current read position offset
            @param position The new read position
         */
        native function set readPosition(position: Number): Void

        /** 
            Read a 16-bit short integer from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof. If there is insufficient data 
            @returns a short int or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readShort(): Number

//  MOB -- 
        /** 
            Read a data from the array as a string. Read data from the $readPosition to a string up to the $writePosition,
            but not more than count characters. If insufficient data, a "writable" event will be issued indicating that 
            the byte array is writable. This enables observers to write data into the byte array.  If there is no data 
            available, the call will return return null indicating eof. If there is insufficient data @param count of bytes 
            to read. If -1, convert the data up to the $writePosition.
            @returns a string or null on eof.
            @throws IOError if an I/O error occurs or a premature eof.
         */
        native function readString(count: Number = -1): String

        /**
            Read an XML document from the array. Data is read from the current read $position pointer.
            @returns an XML document
            @throws IOError if an I/O error occurs or a premature end of file.
         */
        function readXML(): XML
            XML(readString())

        /** 
            Reset the read and $writePosition pointers if there is no available data.
            This is used to rewind the read/write pointers to maximize available buffer space.
         */
        native function reset(): Void

        /** 
            Number of data bytes that the array can store from the $writePosition till the end of the array.
         */
        native function get room(): Number 

        /** 
            Convert the data in the byte array between the $readPosition and writePosition.
            @return A string
         */
        override native function toString(): String 

        /** 
            Uncompress the array
         */
        # FUTURE
        native function uncompress(): Void

        /** 
            Write data to the ByteArray.
            Data is written to the current $writePosition. If the data argument is itself a ByteArray, the available data 
            from the byte array will be copied, ie. the $data byte array will not have its readPosition adjusted. If the 
            byte array is growable, the underlying data storage will grow to accomodate written data. If the data will not
            fit in the ByteArray, the call may return having only written a portion of the data.
            @duplicate Stream.write
         */
        native function write(...data): Number

//  MOB -- should these routines return the number of bytes written?
//  MOB -- function write(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Write a byte to the array. Data is written to the current write $position pointer which is then incremented.
//  MOB -- no such details exist
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeByte(data: Number): Void

        /** 
            Write a short to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeShort(data: Number): Void

        /** 
            Write a double to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeDouble(data: Number): Void

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer which is 
                then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeInteger(data: Number): Void

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer which is 
            then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeLong(data: Number): Void

        /** 
            Current $writePosition offset.
         */
        native function get writePosition(): Number

        /** 
            Set the current write position offset.
            @param position the new write  position
         */
        native function set writePosition(position: Number): Void

        /** 
            Input callback function when read data is required. The input callback should write to the supplied buffer.
            @hide
            @deprecated 1.0.0B3
        */
        # Config.Legacy
        function get input(): Function { return null; }

        /**  
            @hide
            @deprecated 1.0.0B3
         */
        # Config.Legacy
        function set input(callback: Function): Void {
            on("writable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
        }

        /**  
            Output function to process (output) data. The output callback should read from the supplied buffer.
            @param callback Function to invoke when the byte array is full or flush() is called.
                function outputCallback(buffer: ByteArray): Number
            @hide
            @deprecated 1.0.0B3
         */
        # Config.Legacy
        function get output(): Function { return null; } 

        # Config.Legacy
        function set output(callback: Function): Void {
            on("readable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
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
