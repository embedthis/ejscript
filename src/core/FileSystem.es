/*
    FileSystem.es -- FileSystem class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The FileSystem objects provide global information about a file systems.
        @spec ejs
        @stability prototype
     */
    class FileSystem {

        use default namespace public

        //  TODO - getter
        /**
            Get a list of file system drives
            @return An array of drives. Returns an empty list on systems without drive specifiers in filenames.
         */
        native static function drives(): Array 

        /** 
            Create a new FileSystem object for the file system that contains the given path.
            @param path String or Path of a file that would reside in the file system.
         */
        native function FileSystem(path: Path? = null)

        /** 
            Are path names on this file system case sensitive. ie. are uppercase and lowercase path names equivalent
         */
        #FUTURE
        native function get caseSensitive(): Boolean

        /** 
            Do path names on this file system preserve case
         */
        #FUTURE
        native function get casePreserved(): Boolean

        /** 
            Free space in the file system in 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get space(): Number

        /** 
            Do path names on this file system support a drive specifications.
         */
        native function get hasDrives(): Boolean 

        /** 
            Is the file system available, mounted and ready for use
         */
        #FUTURE
        native function get isReady(): Boolean

        /** 
            Is the file system is writable
         */
        #FUTURE
        native function get isWritable(): Boolean

        /** 
            The new line characters for this file system. Usually "\n" or "\r\n".
         */
        native function get newline(): String 

        /** */
        native function set newline(terminator: String): Void

        /** 
            Path to the root directory of the file system
         */
        native function get root(): Path

        /** 
            Path directory separators. The first character is the default separator. Usually "/" or "\\".
         */
        native function get separators(): String 

        /** */
        native function set separators(sep: String): Void 

        /** 
            The total size of the file system in of 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get size(): Number
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
