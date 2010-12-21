/*
    FileSystem.es -- FileSystem class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The FileSystem objects provide statistics and data about file systems.
        @spec ejs
        @stability prototype
     */
    class FileSystem {

        use default namespace public

        /** 
            Create a new FileSystem object based on the given path.
            @param path String or Path of the file system
         */
        native function FileSystem(path: Object? = null)

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
