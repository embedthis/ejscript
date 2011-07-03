/*
    File.es -- File I/O class. Do file I/O and manage files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  MOB - rethink various throws

module ejs {

    /** 
        The File class provides a foundation of I/O services to interact with physical files.
        Each File object represents a single file, a named path to data stored in non-volatile memory. A File object 
        provides methods for creating, opening, reading, writing and deleting a file, and for accessing and modifying 
        information about the file. All I/O is unbuffered and synchronous.
        @spec ejs
        @stability evolving
     */
    final class File implements Stream {

        use default namespace public

        /**
            Async mode is not yet supported
            @hide 
          */
        function get async(): Boolean
            false

        /** 
            Async mode is not yet supported
            @hide 
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("File class does not support async I/O")
            }
        }

        /** 
            Create a File object and open the requested path.
            @param path the name of the file to associate with this file object. Can be either a String or a Path.
            @param options If the open options are provided, the file is opened. See $open for the available options.
         */
        native function File(path: Object, options: Object? = null)

        /** 
            Is the file opened for reading
         */
        native function get canRead(): Boolean

        /** 
            Is the file opened for writing
         */
        native function get canWrite(): Boolean

        /** 
            Close the input stream and free up all associated resources.
         */
        native function close(): void 

        /** 
            Current encoding schem for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String
            "utf-8"

        /** 
            Set the encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            File I/O is currently unbuffered
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Iterate over the positions in a file. This will get an iterator for this file to be used by 
                "for (v in files)"
            @return An iterator object that will return numeric offset positions in the file.
         */
        override iterator native function get(): Iterator

        /** 
            Get an iterator for this file to be used by "for each (v in obj)". Return each byte of the file in turn.
            @return An iterator object that will return the bytes of the file.
         */
        override iterator native function getValues(): Iterator

        /** 
            Is the file open
         */
        native function get isOpen(): Boolean

        /** 
            TODO
            @duplicate Stream.off 
            @hide
         */
        native function off(name, observer: Function): Void

        /** 
            TODO
            @duplicate Stream.on 
            @hide
         */
        native function on(name, observer: Function): Void

        //  MOB - would it be better not to throw?
        /**  
            Open a file. This opens the file designated when the File constructor was called.
            @params options Optional options. If ommitted, the options default to open the file in read mode.
                Options can be either a mode string or can be an options hash. 
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "c" to create the file if it does not exist, "l" to gain an exclusive lock, "s" for a shared lock,
                "t" for text mode, and "+" to never truncate. Defaults to "r". NOTE: not all platforms support "l" and "s".
                If "w" is specified and the file does not exist, it will be created. If "+" is not specified, the file
                    will be truncated when opened, unless "a" is specified to append to existing content.
                If "c" is specified and the file exists, the open will fail.
            @options permissions Number containing the Posix permissions number value. Note: this is a number
                and not a string representation of an octal posix number.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return the File object. This permits method chaining.
            @throws IOError if the path or file cannot be created.
         */
        native function open(options: Object? = null): File

        /** 
            Current file options set when opening the file.
         */ 
        native function get options(): Object

        /** 
            The name of the file associated with this File object or null if there is no associated file.
         */
        native function get path(): Path 

        /** 
            The current read/write I/O position in the file.
         */
        native function get position(): Number

        /** 
            Seek to a new location in the file and set the File marker to a new read/write position.
            @param loc Location in the file to seek to. Set the position to zero to reset the position to the beginning 
            of the file. Set the position to a negative number to seek relative to the end of the file (-1 positions 
            at the end of file).
            @throws IOError if the seek failed.
         */
        native function set position(loc: Number): void

        /** 
            Read a block of data from a file into a byte array. This will advance the read file's position.
                This will read up to $count bytes that will fit into the provided buffer. 
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of bytes to read. If -1, read much as the buffer will hold up to the entire file if the 
                buffer is of sufficient size or is growable.
            @return A count of the bytes actually read. Returns null on end of file.
            @throws IOError if the file could not be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read data bytes from a file and return a byte array containing the data.
            @param count Number of bytes to read. If null, read the entire file.
            @return A byte array containing the read data. Returns an empty array on end of file.
            @throws IOError if the file could not be read.
         */
        native function readBytes(count: Number = -1): ByteArray

        //  MOB UNICODE encoding. Should this do UTF-8 to Unicode-16?
        /** 
            Read data from a file as a string.
            @param count Number of bytes to read. If -1, read the entire file.
            @return A string containing the read data. Returns an empty string on end of file.
            @throws IOError if the file could not be read.
         */
        native function readString(count: Number = -1): String

        /** 
            Remove a file
            @return true if the file could be removed
         */
        function remove(): Boolean {
            if (isOpen) {
                return false
            }
            return Path(path).remove()
        }

        /** 
            The size of the file in bytes.
         */
        native function get size(): Number 

        //  MOB -- perhaps better to not throw
        /**     
            Truncate the file. 
            @param value the new length of the file
            @throws IOError if the file's size cannot be changed
         */
        native function truncate(value: Number): Void 

        /** 
            Write data to the file. All I/O is unbuffered and synchronous. 
            @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
            first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
            the BinaryStream class to control the byte ordering when writing numbers.
            @returns the number of bytes written.
            @throws IOError if the file could not be written.
         */
        native function write(...items): Number
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
