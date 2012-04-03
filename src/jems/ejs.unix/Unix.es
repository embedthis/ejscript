/*
    Unix.es -- Unix compatibility functions

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.unix {

    // use default namespace public

    /**
        Get the base name of a file. Returns the base name portion of a file name. The base name portion is the 
        trailing portion without any directory elements.
        @return A string containing the base name portion of the file name.
     */
    function basename(path: String): Path
        Path(path).basename
    
    //  TODO - should this be cd()
    /**
        Change the current working directory
        @param dir Directory String or path to change to
     */
    function chdir(dir: Object): Void
        App.chdir(dir)

    /**
        Set the permissions of a file or directory
        @param path File or directory to modify
        @param perms Posix style permission mask
     */
    function chmod(path: String, perms: Number): Void
        Path(path).perms = perms

    /**
        Copy files
        @param patterns Pattern to match files to copy. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
            If patterns is an existing directory, then the pattern is converted to 'dir/ * *' (without spaces) 
            and the tree option is enabled.
        @param dest Destination file or directory. If multiple files are copied, dest is assumed to be a directory and 
            will be created if required.
        @param options File attributes
        @options expand Set to hash of properties to use when expanding '${token}' tokens in src or dest filenames.
        @options owner String representing the file owner                                                     
        @options group String representing the file group                                                     
        @options permissions Number File Posix permissions mask
        @options process Optional callback function to process the copied file. This function must do the actual copy 
            and any required post-processing. Signature is function process(src: Path, dest: Path, options)
        @options tree Copy the entire subtree identified by the patterns by prepending the entire subtree path.
        @return Number of files copied
    */
    function cp(patterns, dest: Path, options = {}): Number {
        //  MOB - refactor as it doesn't use recursion anyway
        function inner(path: Path, patterns, dest: Path, options, level: Number): Number {
            let count = 0
            if (options.expand) {
                 dest = dest.toString().expand(options.expand, options)
            }
            let list = path.glob(patterns, options)
            if (list.length > 1 || (patterns is Array && patterns.length > 1)) {
                if (!options.cat) {
                    if (!dest.exists) {
                        dest.makeDir()
                    } else if (!dest.isDir) {
                        throw 'Destination "' + dest + '" is not a directory'
                    }
                }
            }
            for each (let file: Path in list) {
                let from = path.join(file)
                if (options.expand) {
                     file = file.toString().expand(options.expand, options)
                }
                let target
                if (options.tree) {
                    //  Don't use join to allow for file to be absolute
                    target = Path(dest + "/" + file).normalize
                } else if (dest.isDir) {
                    target = dest.join(file.basename)
                } else {
                    target = dest
                }
                target.dirname.makeDir()
                if (options.process) {
                    /* Ensure we get any bound "this" value */
                    let fn = options.process
                    fn(from, target, options)
                } else if (from.isDir) {
                    target.makeDir()
                } else {
                    from.copy(target, options)
                }
                count++
            }
            if (count == 0 && level == 0 && options.warn) {
                throw new ArgError('cp: Can\'t find files for "' + patterns + '" to ' + dest)
            }
            return count
        }
        let path = Path('.')
        if (Path(patterns).isDir) {
            path = Path(patterns)
            patterns = '**'
            if (dest.isDir) {
                dest = dest.join(path.basename)
            }
            options = blend({tree: true, relative: true}, options)
        }
        return inner(path, patterns, dest, options, 0)
    }

    /**
        Get the directory name portion of a file. The dirname name portion is the leading portion including all 
        directory elements and excluding the base name. On some systems, it will include a drive specifier.
        @return A string containing the directory name portion of the file name.
     */
    function dirname(path: String): Path
        Path(path).dirname

    /**
        Does a file exist. Return true if the specified file exists and can be accessed.
        @param path Filename path to examine.
        @return True if the file can be accessed
     */
    function exists(path: String): Boolean
        Path(path).exists

    /**
        Get the file extension portion of the file name. The file extension is the portion after the last "."
        in the path.
        @param path Filename path to examine
        @return String containing the file extension. It excludes "." as the first character.
     */
    function extension(path: String): String
        Path(path).extension

    /**
        Return the free space in the file system.
        @return The number of 1M blocks (1024 * 1024 bytes) of free space in the file system.
     */
    function freeSpace(path: String? = null): Number
        FileSystem(path).freeSpace()

    /**
        Is a file a directory. Return true if the specified path exists and is a directory
        @param path Directory path to examine.
        @return True if the file can be accessed
     */
    function isDir(path: String): Boolean
        Path(path).isDir

    /**
        Kill a process
        @param pid Process ID to kill
        @param signal Signal number to use when killing the process.
        @hide
        @deprecated 2.0.0
     */
    function kill(pid: Number, signal: Number = 2): Void 
        Cmd.kill(pid, signal)

    /**
        Get a list of files in a directory.
        @param patterns Pattern to match files. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options If set to true, then files will include sub-directories in the returned list of files.
        @option dirs Include directories in the file list
        @option depthFirst Do a depth first traversal. If "dirs" is specified, the directories will be shown after
        the files in the directory. Otherwise, directories will be listed first.
        @option exclude Regular expression pattern of files to exclude from the results. Matches the entire path.
        @option hidden Show hidden files starting with "."
        @option include Regular expression pattern of files to include in the results. Matches the entire returned path.
        @return An Array of Path objects for each matching file.
     */
    function ls(patterns = "*", options: Object = null): Array {
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        let results = []
        for each (let pat: Path in patterns) {
            if (pat.exists && pat.isDir) {
                pat = pat.join("*")
            }
            results += Path(".").glob(pat, options)
        }
        return results
    }


    /**
        Find files. Do Posix glob style pattern matching.
        @param pattern String pattern to match with files. The wildcards "*", "**" and "?" are the wild card
            patterns supported. The "**" pattern matches any number of directories.  The Posix "[]" and "{a,b}" style
            expressions are not supported.
        @param options If set to true, then files will include sub-directories in the returned list of files.
        @option dirs Include directories in the file list
        @option depthFirst Do a depth first traversal. If "dirs" is specified, the directories will be shown after
        the files in the directory. Otherwise, directories will be listed first.
        @option exclude Regular expression pattern of files to exclude from the results. Matches the entire path.
        @option hidden Show hidden files starting with "."
        @option include Regular expression pattern of files to include in the results. Matches the entire returned path.
        @return An Array of Path objects for each matching file.
     */
    function find(path: Object, pattern: String = "*", options = {}): Array {
        let result = []
        if (path is Array) {
            let paths = path
            for each (path in paths) {
                result += Path(path).glob(pattern, options)
            }
        } else {
            result += Path(path).glob(pattern, options)
        }
        return result
    }

    /**
        Make a new directory. Makes a new directory and all required intervening directories. If the directory 
        already exists, the function returns without throwing an exception.
        @param path Filename path to use.
        @param permissions Optional posix permissions mask number. e.g. 0664.
        @throws IOError if the directory cannot be created.
     */
    function mkdir(path: String, permissions: Number = 0755): void
        Path(path).makeDir({permissions: permissions})
    
    /**
        Rename a file. If the new file name exists it is removed before the rename.
        @param fromFile Original file name.
        @param toFile New file name.
        @throws IOError if the original file does not exist or cannot be renamed.
     */
    function mv(fromFile: String, toFile: String): void
        Path(fromFile).rename(toFile)

    /**  
        Open or create a file
        @param path Filename path to open
        @param mode optional file access mode with values values from: Read, Write, Append, Create, Open, Truncate. 
            Defaults to Read.
        @param permissions optional permissions. Defaults to App.permissions
        @return a File object which implements the Stream interface
        @throws IOError if the path or file cannot be opened or created.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function open(path: String, mode: String = "r", permissions: Number = 0644): File
        new File(path, { mode: mode, permissions: permissions})

    /**
        Get the current working directory
        @return A Path containing the current working directory
     */
    function pwd(): Path
        App.dir

    /**  
        Read data bytes from a file and return a byte array containing the data.
        @param file Open file object previously opened via $open or $File
        @param count Number of bytes to read
        @return A byte array containing the read data
        @throws IOError if the file could not be read.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function read(file: File, count: Number): ByteArray
        file.read(count)

    /**
        Remove files from the file system.
        @param patterns Pattern to match files to remove. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options Options to modify the removal
        @option exclude Regular expression pattern of files to exclude from removal. Matches the entire path.
        @option hidden Remove hidden files starting with "." Defaults to true.
        @option include Regular expression pattern of files to remove. Matches the entire returned path.
        @option nodirs Exclude directories from removal. The default is to include directories.
        @return True if all the contents are sucessfully deleted. Otherwise return false.
    */
    function rm(patterns, options = {}): Boolean {
        options = blend({depthFirst: true, hidden: true}, options)
        let success = true
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        for each (let pat:Path in patterns) {
            for each (let path: Path in Path('.').glob(pat, options)) {
                if (!path.remove()) {
                    success = false
                }
            }
        }
        return success
    }

    /**
        Removes a directory and contents
        @param patterns Pattern to match files to delete. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options Options to modify the removal
        @option exclude Regular expression pattern of files to exclude from removal. Matches the entire path.
        @option hidden Remove hidden files starting with "."
        @option include Regular expression pattern of files to remove. Matches the entire returned path.
        @return True if all the contents are sucessfully deleted or if the directory does not exist. Otherwise return false.
     */
    function rmdir(patterns, options = {}): Boolean {
        options = blend({depthFirst: true, hidden: true}, options)
        let success = true
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        for each (let pat:Path in patterns) {
            for each (let path: Path in Path('.').glob(pat)) {
                if (path.isDir) {
                    rmdir(path.join('*'), options)
                }
                if (!path.remove()) {
                    success = false
                }
            }
        }
        return success
    }

    /** 
        Sleep the application for the given number of milliseconds. Events will be serviced while asleep.
        @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
     */
    function sleep(delay: Number): void
        App.sleep(delay)

    /**
        Create a temporary file. Creates a new, uniquely named temporary file.
        @param directory Directory in which to create the temp file.
        @returns a closed File object after creating an empty temporary file.
     */
    function tempname(directory: String? = null): File
        Path(directory).makeTemp()

    /**
        Write data to the file. If the stream is in sync mode, the write call blocks until the underlying stream or 
        endpoint absorbes all the data. If in async-mode, the call accepts whatever data can be accepted immediately 
        and returns a count of the elements that have been written.
        @param file Open file object previously opened via $open or $File
        @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
        first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
        the BinaryStream class to write Numbers.
        @returns the number of bytes written.  
        @throws IOError if the file could not be written.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function write(file: File, ...items): Number
        file.write(items)
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
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
