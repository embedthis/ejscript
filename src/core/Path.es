/*
    Path.es --  Path class. Path's represent files in a file system. The file may or may not exist.  

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Paths are filename objects and may or may not represent physical files in a file system. That is, the 
        corresponding file or directory for the Path  may or may not exist. Once created, Paths are immutable and their
        path value cannot be changed.
        @spec ejs
        @stability evolving
     */
    final class Path {

        use default namespace public

        /**
            Create a new Path object and set the path's location.
            @param path Name of the path to associate with this path object. 
         */
        native function Path(path: String = '.')

        /**
            An equivalent absolute path equivalent for the current path. The path is normalized and uses the native system
            directory separator.
            On Windows, this path with contain a drive specifier and will have back-slash directory separators.
            On Cygwin, this path will be a Cygwin style path without a drive specifier and with forward-slash 
            directory separators. If the path is outside the Cygwin filesystem (outside c:/cygwin), the path 
            will have a /cygdrive/DRIVE prefix. 
            To get a Windows style path, use the $windows property accessor.
         */
        native function get absolute(): Path

        /**
            When the file represented by the path was last accessed. Set to null if the file does not exist.
         */
        native function get accessed(): Date?

        /**
            Append data to a file.
            @param data Data to append to the file
            @param options File open options. Defaults to 'atw'
         */
        function append(data, options = 'atw'): Void {
            let file = open(options)
            file.write(data)
            file.close()
        }

        /**
            File security permissions.
            @return the file attributes object hash. Fields include: 
            @options user String representing the file user name
            @options group String representing the file group name
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @options permissions Number File Posix permissions mask
         */
        native function get attributes(): Object

        /**
            Define file security permissions.
            @options user String representing the file user name If both user and uid are specified, user takes precedence. 
            @options group String representing the file group name If both group and gid are specified, group takes
                precedence.  
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @options permissions Number File Posix permissions mask
         */
        native function setAttributes(attributes: Object): Void

        /**
            The base of portion of the path. The base portion is the trailing portion without any directory elements.
         */
        native function get basename(): Path

        /**
            Test if the path is a child of a given directory
            @param dir Path Parent directory to test
            @return true if the Path is a child of the specified parent directory.
         */
        function childOf(dir: Path): Boolean
            absolute.startsWith(dir.absolute)
        
        /**
            Path components. The path is normalized and then broken into components for each directory level. 
            The result is an array object with an element for each segment of the path. If the path is an absolute path, 
            the first element represents the file system root and will be the empty string or drive spec 
            on a Windows like systems. Array.join('/') can be used to put components back into a complete path.
         */
        native function get components(): Array
  
        /**
            Test if the path name contains a substring. The pattern must be in portable format.
            @param pattern String pattern to search for
            @return Boolean True if found.
         */
        function contains(pattern: String): Boolean
            portable.name.contains(pattern)

        /**
            Copy a file to the destination
            @param destination New file location. If the destination is a directory or ends with the file separator ('/'), 
                the file is copied to that directory using the basename portion of the source filename.
            @param options Object hash
            @options permissions Set to a numeric Posix permissions mask. Not implemented.
            @options user String representing the file user name or numeric user id.
                If both user and uid are specified, user takes precedence. 
            @options group String representing the file group name or numeric group id.
                If both group and gid are specified, group takes precedence.
         */
        native function copy(destination: Object, options: Object? = null): Void

        /**
            When the file represented by the path was created. Set to null if the file does not exist.
         */
        native function get created(): Date?

        /**
            The directory portion of a file. The directory portion is the leading portion including all 
            directory elements and excluding the base name. On some systems, it will include a drive specifier.
            See also $parent which will determine the parent directory of relative paths.
         */
        native function get dirname(): Path

        /**
            Return true if the path ends with the given suffix
            @param suffix String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: String): Boolean
            portable.name.endsWith(Path(suffix).portable)

        /**
            Does a file exist for this path.
         */
        native function get exists(): Boolean 

        /**
            File extension portion of the path. The file extension is the portion after (and not including) the last '.'.
            Returns empty string if there is no extension
         */
        native function get extension(): String 

        /**
            TODO - should do pattern matching
            @hide
         */
        function findAbove(name: String): Path? {
            let dir: Path = this
            do {
                if (dir.join(name).exists) {
                    return dir.join(name)
                }
                dir = dir.parent
            } while (dir != dir.parent)
            return null
        }

        /**
            Get a list of matching files. This does
            Posix style glob file matching on supplied patterns and returns an array of matching files.

            This method supports several invocation forms:
            <ul>
                <li>files(pattern, {options})</li>
                <li>files([pattern, pattern, ...], {options})</li>
                <li>files({files: pattern, options...})</li>
                <li>files([{files: pattern, options...}])</li>
            </ul>

            A single pattern may be supplied with or with out options. Multiple patterns may be provided in an array. 
            Alternatively, the pattern or patterns may be provided via an object with a 'files' property. In this case, the
            options are provided in the same object. If the pattern is omitted, the default is to search for all files
            under the Path by using '**' as the pattern.

            @param patterns Pattern to match files. This can be a String, Path or array of String/Paths. 
            If the Path is a directory, then files that match the specified patterns are returned.
            If the Path is not a directory, then the path itself is matched against the pattern.
            The following special sequences are supported:
            <ul>
                <li>The wildcard '?' matches any single character</li>
                <li>* matches zero or more characters in a filename or directory</li>
                <li>** matches zero or more files or directories and matches recursively in a directory tree</li>
                <li>! Negates pattern. This removes matching patterns from the set. These are applied after all source
                      patterns have been processed. Use !! to escape or set noneg in options.
            </ul>
            If a pattern ends with '/', then all the directory contents will match. 

            @param options Optional properties to control the matching.
            @option contents Boolean If contents is set to true and the path pattern matches a directory, then return the
                contents of the directory in addition to the directory itself. This applies if the pattern matches an 
                existing directory or the pattern ends with '/'. This is implemented by appending '**' to the pattern.

            @option depthFirst Boolean Do a depth first traversal of directories. If true, then the directories will be 
                shown after the files in the directory. Otherwise, directories will be listed first. Defaults to false.

            @option directories Boolean Include directories in the results. Defaults to true.

            @option exclude String | Regular | Function Blacklist of files to exclude from the results. 
                Matches the full source path including the Path value. Only for the purpose of this match, 
                directories will have '/' appended.

                If 'exclude' is a function, the file is not processed if the function returns true. The function callback
                has the form:

                Function(filename: Path, options: Object): Boolean

                The file argument is the filename being considered, it include the Path value.
                The value of "this" is set to the Path value.

                By default, exclude is set to exclude hidden files starting with '.' and files that look like temporary 
                files. 

            @option expand Object | Function Expand tokens in filenames using this object or function. If 'expand' 
                is an object, then expand filename tokens of the form '${token}' using the object key values.
                If 'expand' is a function, it should have the signature:

                Function(filename: String, options: Object): String

            @option hidden Boolean Include hidden files starting with '.' Defaults to false.

            @option include String | RegExp | Function Whitelist of files to include in the results.
                Matches the full source path including the Path value. Only for the purpose of this match, 
                directories will have '/' appended.

                If 'include' is a function, the file is processed if the function returns true. The function callback
                has the form:

                Function(filename: Path, options: Object): Boolean

                The file argument is the filename being considered, it include the Path value.
                The value of "this" is set to the Path value.

            @option missing String Determine what happens if a pattern does not match any files.
                Set to undefined to report patterns that don't resolve by throwing an exception. 
                Set to any non-null value to be used in the results in place of the pattern when there are no matching 
                    files or directories. 
                Set to the empty string to use the pattern itself in the results.
                Set to null to do nothing and omit the pattern.
                Default is null.

            @option noneg Boolean Do not process negated file patterns.

            @option relative Boolean | String | Path Return filesnames relative to the path provided by the 'relative'
                option, otherwise results include the Path value. If set to true, the results will be relative to the 
                current directory. Defaults to false. 

            @return An Array of Path objects for each file in the directory.
                The returned file list includes the Path itself. If the 'relative' option is provided, a path
                relative to that option is returned.
         */
        native function files(patterns: Object! = '*', options: Object? = null): Array 

        /**
            The file system containing this Path 
            @return The FileSystem object for this path
         */
        function get fileSystem(): FileSystem
            FileSystem(this)

        /**
            Get iterate over any files contained under this path (assuming it is a directory) 'for (v in files)'. 
                This operates the same as getValues on a Path object.
            @return An iterator object.
            @example:
                for (f in Path('.'))
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this file to be used by 'for each (v in obj)'. Return 
                This operates the same as $get on a Path object.
            @return An iterator object.
            @example
                for each (f in Path('.'))
         */
        override iterator native function getValues(): Iterator

        /**
            Test if the path matches a 'glob' style pattern
            @return True if the path matches the pattern.
            @hide
         */
        native function glob(pattern: String): Boolean

        /**
            Does the file path have a drive spec (C:) in it's name. Only relevant on Windows like systems.
            @return True if the file path has a drive spec
         */
        native function get hasDrive(): Boolean 

        /**
            Is the path absolute, i.e. Beginning at the file system's root.
         */
        native function get isAbsolute(): Boolean

        //  TODO - need isExecutable
        /**
            Is the path a directory. If the path is a symbolic link, the target of the link is tested.
            @return true if the file is a directory
         */
        native function get isDir(): Boolean

        /**
            Is the path is a symbolic link file. Not available on some platforms such as Windows and VxWorks.
            @return true if the file is a symbolic link
         */
        native function get isLink(): Boolean

        /**
            Is the path a regular file. If the path is a symbolic link, the target of the link is tested.
            @return true if the file is a regular file and not a directory
         */
        native function get isRegular(): Boolean

        /**
            Is the path is relative and not absolute.
         */
        native function get isRelative(): Boolean

        /**
            Join paths. Joins one more more paths together. If the other path is absolute, return it. Otherwise
            add a separator, and continue joining. The separator is chosen to match the first separator found in 
            either path. If none found, the default file system path separator is used.
            @return A new joined, normalized Path object.
         */
        native function join(...other): Path

        /**
            Join an extension to a path. If the base name of the path already has an extension, this call does nothing.
            @param ext Extension to add. The extension may or may not contain a leading period. Best practice is to 
                not have leading periods on extensions.
            @param force Set to true to always add the extension.
            @return A new Path object with the given extension if the path did not already have one.
         */
        native function joinExt(ext: String, force: Boolean = false): Path

        /**
            The length of the path name in bytes. Note: this is not the file size. For that, use Path.size
         */
        native function get length(): Number 

        /**
            Create a target link to refer to the path
            This will remove any pre-existing target link and then create a symbolic link at the target to refer to the
            path.
            @param hard Set to true to create a hard link. Otherwise the default is to create a symbolic link.
            @param target Target the path will refer to.
          */
        native function link(target: Path, hard: Boolean = false): Void

        /**
            The target pointed to if this path is a symbolic link. Not available on some platforms such as Windows and 
            VxWorks. If the path is not a symbolic link, it is set to null.
         */
        native function get linkTarget(): Path?

        /**
            Make a new directory and all required intervening directories. If the directory already exists, 
                the function returns without any further action. 
            @param options
            @options permissions Directory permissions to use for all created directories. Set to a numeric 
                Posix permissions mask.
            @options user String representing the file user.
                If both user and uid are specified, user takes precedence. 
            @options group String representing the file group.
                If both group and gid are specified, group takes precedence.
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @return True if the directory can be made or already exists
         */
        native function makeDir(options: Object? = null): Boolean

        /**
            Create a link to a file. Not available on some platforms such as Windows and VxWorks.
            @param target Path to an existing file to link to.
            @param hard Set to true to create a hard link. Otherwise the default is to create a symbolic link.
            @returns this path
            @hide
            @deprecate 2.3.0
         */
        native function makeLink(target: Path, hard: Boolean = false): Void

        /**
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            If the path object is created with an empty path value, the temporary file will be created in the standard
            system location for temp files. This is typically /tmp or /temp.
            @returns a new Path object for the temp file.
         */
        native function temp(): Path

        /**
            Get a path after mapping the path directory separator
            @param sep Path directory separator to use. Defaults to the separator for this path.
            @return a new Path after mapping separators
         */
        native function map(sep: String = separator): Path

        /**
            Match the path to a regular expression pattern
            @param pattern The regular expression pattern to search for
            @return Returns an array of matching substrings in the path
         */
        function match(pattern: RegExp): Array?
            this.toString().match(pattern)

        /** 
            Get the mime type for a path or extension string. The path's extension is used to lookup the corresponding
            mime type.
            @returns The mime type string
         */
        native function get mimeType(): String

        /**
            When the file represented by the path was last modified. Set to null if the file does not exist.
         */
        native function get modified(): Date?

        /**
            Name of the Path as a string. This is the same as $toString().
         */
        native function get name(): String 

        /**
            Natural (native) respresentation of the path for the platform. This uses the platform file system path 
            separator, this is '\' on Windows and '/' on unix and Cygwin. The returned path is normalized. 
            See also $portable for a portable representation.
         */
        native function get natural(): Path 

        /**
            Normalized representation of the path. This removes 'segment/..' and './' components. Separators are made 
            uniform by converting all separators to be the same as the first separator found in the path. Note: the 
            result is not converted to an absolute path.
         */
        native function get normalize(): Path

        /**
            Open a path and return a File object.
            @param options
            @options mode optional file access mode string. Use 'r' for read, 'w' for write, 'a' for append to existing
                content, '+' never truncate.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a File object which implements the Stream interface.
            @throws IOError if the path or file cannot be created.
         */
        function open(options: Object? = null): File
            new File(this, options)

        /**
            Open a file and return a TextStream object.
            @param options Optional options object
            @options mode optional file access mode string. Use 'r' for read, 'w' for write, 'a' for append to existing
                content, '+' never truncate.
            @options encoding Text encoding format
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a TextStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openTextStream(options: Object? = null): TextStream
            new TextStream(open(options))

        /**
            Open a file and return a BinaryStream object.
            @param options Optional options object
            @options mode optional file access mode with values ored from: Read, Write, Append, Create, Open, Truncate. 
                Defaults to Read.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a BinaryStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openBinaryStream(options: Object? = null): BinaryStream
            new BinaryStream(open(options))

        /**
            The parent directory of path. This may be an absolute path if there are no parent directories in the path.
         */
        native function get parent(): Path

        /*  TODO -- different to File.permissions. Should have something that returns an object with full path/file
            attributes including group/user. The perms should be broken down into world:group:user */
        //  TODO - should not use a setter should be functions
        /**
            The file permissions of a path. This number contains the Posix style permissions value or null if the file 
            does not exist. NOTE: this is not a string representation of an octal posix mask. 
         */
        native function get perms(): Number?
        native function set perms(perms: Number): Void

        /**
            The path in a portable (like Unix) representation. This uses '/' separators. The value is is normalized and 
            the separators are mapped to '/'. See also $natural for convertion to the O/S native path representation.
         */
        native function get portable(): Path 

        /**
            Read the file contents as a ByteArray. This method opens the file, reads the contents, closes the file and
                returns a new ByteArray with the contents.
            @return A ByteArray
            @throws IOError if the file cannot be read
            @example:
                var b: ByteArray = Path('/tmp/a.txt').readBytes()
         */
        function readBytes(): ByteArray? {
            let file: File = File(this).open()
            result = file.readBytes()
            file.close()
            return result
        }

        /**
            Read the file containing a JSON string and return a deserialized object. 
            This method opens the file, reads the contents, deserializes the object and closes the file.
            @return An object.
            @throws IOError if the file cannot be read
            @example:
                data = Path('/tmp/a.json').readJSON()
         */
        function readJSON(): Object? {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return deserialize(result)
        }

        /**
            Read the file contents as an array of lines. Each line is a string. This method opens the file, reads the 
                contents and closes the file.
            @return An array of strings.
            @throws IOError if the file cannot be read
            @example:
                for each (line in Path('/tmp/a.txt').readLines())
         */
        function readLines(): Array? {
            let stream: TextStream = TextStream(open(this))
            result = stream.readLines()
            stream.close()
            return result
        }

        /**
            Read the file contents as a string. This method opens the file, reads the contents and closes the file.
            @return A string.
            @throws IOError if the file cannot be read
            @example:
                data = Path('/tmp/a.txt').readString()
         */
        function readString(): String? {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return result
        }

        /**
            Read the file contents as an XML object.  This method opens the file, reads the contents and closes the file.
            @return An XML object
            @throws IOError if the file cannot be read
         */
        function readXML(): XML? {
            let file: File = open(this)
            let data = file.readString()
            file.close()
            if (data == null) {
                return null
            }
            return new XML(data)
        }

        /**
            That path in a form relative to the application's current working directory. The value is normalized.
         */
        native function get relative(): Path

        /**
            Get a relative path to an origin
            @param origin The origin path to use when calculating a relative path to the destination (this path).
                If origin is null, the current working directory is used as the origin.
            @return A new relative path
         */
        native function relativeTo(origin: Path? = null): Path

        /**
            Delete the file associated with the Path object. If this is a directory without contents it will be removed.
            @return True if the file is sucessfully deleted or does not exist.
         */
        native function remove(): Boolean 

        //  TODO - consider remove({contents: true})
        /**
            Removes a directory and its contents
            If the path is a directory, this call will remove all subdirectories and their contents and finally the
            directory itself. If the directory does not exist, this call does not error and does nothing.
            @return True if all the contents are sucessfully deleted or if the directory does not exist. 
                Otherwise return false.
         */
        function removeAll(): Boolean {
            let passed = true
            if (name == '' || name == '/') {
                throw new ArgError('Bad path for removeAll')
            }
            for each (f in files('**', {depthFirst: true, hidden: true})) {
                if (!f.remove()) {
                    passed = false
                }
            }
            if (!remove()) {
                passed = false
            }
            return passed
        }

        /**
            Remove the drive specifier from a path
            @hide
         */
        function removeDrive(): Path {
            let sep = separator
            return hasDrive ? (sep + components.slice(1).join(sep)) : this
        }

        /**
            Rename a file. If the new path exists it is removed before the rename.
            @param target New name of the path
            @throws IOError if the rename fails.
         */
        native function rename(target: Path): Void
        
        /**
            Replace the path extension and return a new path.
            @return A new path with the given extension.
         */
        function replaceExt(ext: String): Path {
            if (ext && ext[0] != '.') {
                ext = '.' + ext
            }
            return this.trimExt() + ext
        }

        /**
            Search and replace. Search for the given pattern which can be either a string or a regular expression 
            and replace it with the replace text. If the pattern is a string, only the first occurrence is replaced.
            @param pattern The regular expression or string pattern to search for.
            @param replacement The string to replace the match with or a function to generate the replacement text. The
                replacement string can contain special replacement patterns: '$$' inserts a '\$', '\$&' inserts the
                matched substring, '\$`' inserts the portion that preceeds the matched substring, '\$'' inserts the
                portion that follows the matched substring, and '\$N' inserts the Nth parenthesized substring.
                The replacement parameter can also be a function which will be invoked and the function return value 
                will be used as the resplacement text. The function will be invoked multiple times for each match to be 
                replaced if the regular expression is global. The function will be invoked with the signature:

                function (matched, submatch_1, submatch_2, ..., matched_offset, original_string)
            @return Returns a new string.
            @spec ejs
         */
        function replace(pattern, replacement): Path
            Path(name.replace(pattern, replacement))

        /**
            Resolve paths in the neighborhood of this path. Resolve operates like join, except that it joins the 
            given paths to the directory portion of the current ('this') path. For example: 
            Path('/usr/bin/ejs/bin').resolve('lib') will return '/usr/lib/ejs/lib'. i.e. it will return the
            sibling directory 'lib'.

            Resolve operates by determining a virtual current directory for this Path object. It then successively 
            joins the given paths to the directory portion of the current result. If the next path is an absolute path, 
            it is used unmodified.  The effect is to find the given paths with a virtual current directory set to the 
            directory containing the prior path.

            Resolve is useful for creating paths in the region of the current path and gracefully handles both 
            absolute and relative path segments.
            @param otherPaths Paths to resolve in the region of this path.
            @return A new Path object that is resolved against the prior path. 
         */
        native function resolve(...otherPaths): Path

        /**
            That root directory component of the path. On windows, this includes the drive spec if present in the path.
          */
        native function get root(): Path

        /**
            Compare two paths test if they represent the same file
            @param other Other path to compare with
            @return True if the paths represent the same underlying filename
         */
        native function same(other: Object): Boolean

        /**
            The path separator for this path. This will return the first path separator used by the path
            or the default file system path separator if the path has no separators. On Windows, a path may use
            '/' or '\' separators. Use $natural, $portable or $windows to create a new path with different path separators.
         */
        native function get separator(): String

        /**
            Size of the file associated with this Path object. Set to the number of bytes in the file or -1 if the
            size determination failed.
         */
        native function get size(): Number 

        /**
            Return true if the path starts with the given prefix
            @param prefix String prefix to compare with the path.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: String): Boolean
            portable.name.startsWith(Path(prefix).portable)

        //  TODO - symlink is backwards
        /**
            Create the path as a symbolic link to refer to the target
            This will remove any pre-existing path and then create a symbolic link at path to refer to the target.
            @param target Target the path will refer to.
            @hide
            @deprecate 2.3.0
          */
        native function symlink(target: Path): Void

        /**
            Convert the path to a JSON string. 
            @return a JSON string representing the path.
         */
        native override function toJSON(): String

        /**
            Convert the path to lower case
            @return a new Path mapped to lower case
         */
        function toLowerCase(): Path
            new Path(name.toString().toLowerCase())

        /**
            Return the path as a string. The path is not translated.
            @return a string representing the path.
         */
        native override function toString(): String

        /**
            Operate on files.  This may be used to copy, move or process files.

            This method supports several invocation forms:
            <ul>
                <li>operate(from, to)</li>
                <li>operate(from, to, {options})</li>
                <li>operate([from, from, ...], to, {options})</li>
                <li>operate({from: pattern, to: directory, options...})</li>
                <li>operate([{from: pattern, to: directory, options...}])</li>
            </ul>
            The 'options' may supply a desired 'operation' property that may be set to 'copy', 'move', 'append' or custom
            action. This defaults to 'copy'.

            The 'from' patterns are resolved using the #Path.files method to create an input files list. 
            All the options for Path.files are supported.

            Operate will resolve the input patterns relative to the Path itself. If copying, that directory portion relative
            to the Path will be appended to the destination 'to' argument. In this manner, entire directory trees can be 
            easily copied.

            @return The number of files processed. If the operation is 'list', returns the expanded source file list.

            @param from Path | String | Object | Array This may be a String or Path containing the source paths to 
                process. Alternatively, it may be an object that supplies 'from', 'to' and 'options' as processing 
                instructions. It may also be an array of Paths, Strings or Objects. The 'from' patterns may contain:
                    *
                    **
                    ?
                    !   Negates pattern. This removes matching patterns from the set. These are applied after all source
                        patterns have been processed. Use !! to escape.
                If item is a directory or ends with '/', then "**" is automatically appended to match the directory 
                contents.

            @param to String | Path Destination target. If 'from' is a Path or String, then the 'to' argument 
                must be a destination Path or String. If 'from' is an Object or Array of instruction objects that contains
                a 'to' property, then 'to' should be omitted and will be ignored. If multiple source files are specified 
                or the destination ends in the separator '/', the destination is assumed to be a directory. If the 
                destination is a directory, the destination filename is created by appending the the source path to the 
                directory.

            @param control Additional processing instructions. All the options provided by #Path.files are 
                also supported.

            @option active Boolean If destination is an active executable or library, rename the active file using a '.old' 
                extension and retry.

            @option compress Boolean Compress destination file using Zlib. Results in a '.gz' extension.

            @option divider String Divider text to use between appended files.

            @option extension String | Path Extension to use for the destination filenames.

            @option extensionDot String Specifies where the filename extension begins for filenames with multiple dots. 
                Set to 'first' or 'last'.

            @option filter RegExp Pattern of lines to filter out from appended files.

            @option flatten Boolean Flatten the input source tree to a single level. Defaults to true.

            @option footer String Footer to append when appending files.

            @option group String | Number System group name or number to use for the destination files.

            @option header String Header prepend when appending files.

            @option isDir Assume the destination is a directory. Create if it does not exist. Same as appending a 
                trailing '/' to the 'to' argument.

            @option keep Boolean Keep uncompressed file after compressing.

            @option operation String Set to 'append' to append files, 'copy' to copy files and 'move' to move files.
                Set to 'list' to return a file list in options.list and perform no operations. Defaults to 'copy' if unset.

            @option patch Object. Expand file contents tokens using this object. Object hash containing properties to use 
                when replacing tokens of the form ${token} in file contents.

            @option perform Function Callback function to perform the operation on a matching file. 
                This function should return true if it handles the file and default processing is not required.
                Note that post processing still occurs including compression and stripping if required regardless of the 
                return value.

                Function(from: Path, to: Path, options: Object): Boolean

            @option permissions Number Posix style permissions mask. E.g. 0644.

            @option postPerform Function. Callback function to invoke after performing the operation.
                Function(from, to, options): Path

            @option prePerform Function. Callback function to invoke before performing the operation.
                Function(from, to, options): Path

            @option relative Boolean | String | Path Create destination filenames relative to the path provided by the 
                'relative' option, otherwise destination filenames include the Path value. If set to true, the destination
                 will be relative to the current directory. If set, implies flatten == false. Defaults to false. 

            @option rename Function Callback function to provide a new destination filename. 
                Function(from, to, options): Path

            @option strip Boolean Run 'strip' on the destination files.

            @option symlink String | Path Create a symbolic link to the destination. If symlink has a trailing '/'
                a link is created in the directory specified by 'symlink' using the source file basename as the link name.

            @option trim Number of path components to trim from the start of the source filename. 
                If set, implies flatten == false.

            @option user String | Number System user account name or number to use for destination files.

            @option verbose true | function. If true, then trace to stdout. Otherwise call function for each item.
        */
        function operate(from, to, control) {
            let instructions
            control ||= {}
            if (to == undefined) {
                instructions = from
            } else {
                instructions = [ blend({ from: from, to: to }, control, {functions: true, overwrite: false}) ]
            }
            if (!(instructions is Array)) {
                instructions = [instructions]
            }
            let trace = control.verbose
            if (trace && !(trace is Function)) {
                trace = function(...args) {
                    let msg = '%12s %s' % (['[' + args[0] + ']'] + [args.slice(1).join(' ')]) + '\n'
                    stdout.write(msg)
                }
            }
            let expand = control.expand
            if (expand) {
                if (!(expand is Function)) {
                    let obj = expand
                    expand = function(str, obj) str.expand(obj, {missing: true})
                }
            } else {
                expand = function(str, obj) str
            }
            let srcHash = {}
            let commands = []

            for each (options in instructions) {
                if (options.append) {
                    options.operation = 'append'
                }
                if (options.move) {
                    options.operation = 'move'
                }
                if (options.flatten == undefined) {
                    options.flatten = true
                }
                //  LEGACY 
                if (options.cat) {
                    print('Warn: using legacy "cat" property for Path.operate, use "append" instead.')
                    options.append = options.cat
                }
                if (options.remove) {
                    print('Warn: using legacy "remove" property for Path.operate, use "filter" instead.')
                    options.filter = options.remove
                }
                if (options.separator) {
                    print('Warn: using legacy "separator" property for Path.operate, use "divider" instead.')
                    options.divider = options.separator
                }
                //  LEGACY
                if (options.title) {
                    print('Warn', 'Manifest using legacy "title" property, use "header" instead')
                }
                let operation = options.operation || 'copy'
                let from = options.from
                if (!(from is Array)) from = [from]
                from = from.transform(function(e) Path(expand(e, options)))

                /*
                    Do not sort this list as we want to preserve user supplied order in their files list
                    The list files are relative to 'this'
                 */
                let files = this.files(from, blend({contents: true, relative: this}, options, {overwrite: false}))

                let to = Path(expand(options.to, options))
                let sep = to.separator
                let toDir = to.isDir || to.name.endsWith(sep) || options.isDir
                if (operation != 'append') {
                    toDir ||= from.length > 1 || files.length > 1
                }
                if (operation == 'list') {
                    let list = []
                    for each (let file: Path in files) {
                        let src = this.join(file)
                        if (src.isDir && options.flatten) {
                            continue
                        }
                        if (srcHash[src]) {
                            continue
                        }
                        list.push(this.join(file))
                        srcHash[src] = true
                    }
                    return list
                }
                /* 
                    Process matching files and build list of commands
                    Note: 'src' is relative to 'this' and may not be addressible unless 'this' is '.'
                 */
                for each (let file: Path in files) {
                    let src = this.join(file)
                    let dest
                    if (toDir) {
                        if (options.relative) {
                            let trimmed = file
                            if (options.trim) {
                                trimmed = trimmed.components.slice(options.trim).join(to.separator)
                            }
                            if (options.relative == true) {
                                dest = to.join(this.join(trimmed).relative)
                            } else {
                                dest = to.join(this.join(trimmed).relativeTo(options.relative))
                            }

                        } else if (options.trim) {
                            dest = to.join(file.components.slice(options.trim).join(to.separator))

                        } else if (options.flatten) {
                            dest = to.join(file.basename)

                        } else if (file.isAbsolute) {
                            /* Can happen if the from pattern is absolute */
                            dest = Path(to.name + file.name)

                        } else {
                            dest = to.join(file)
                        }
                    } else {
                        dest = to
                    }
                    if (options.extension) {
                        if (options.extensionDot == 'first') {
                            dest = dest.split('.')[0].joinExt(options.extension)
                        } else {
                            dest = dest.replaceExt(options.extension)
                        }
                    }
                    if (src.isDir && options.flatten) {
                        continue
                    }
                    if (options.rename) {
                        dest = options.rename.call(this, src, dest, options)
                    }
                    if (srcHash[src]) {
                        continue
                    }
                    if (src.same(dest) && operation != list) {
                        print("WARNING: src same as dest for", src)
                        continue
                    }
                    commands.push({base: this, from: file, to: dest, options: options})
                    srcHash[src] = true
                }
                if (options.debug) {
                    dump('Operate File List', commands)
                }
                if (operation == 'commands') {
                    return commands
                }
                let contents = []
                if (operation == 'append') {
                    //  LEGACY
                    if (options.title) {
                        options.header = '/* ' + options.title + ' */\n' + options.header
                    }
                    if (options.header) {
                        contents.push(expand(options.header, options) + '\n')
                    }
                }
                /*
                    Process all qualifying files
                 */
                for each (item in commands) {
                    let src = this.join(item.from)
                    let dest = item.to
                    trace && trace(operation.toPascal(), src + ' => ' + dest)

                    if (options.prePerform) {
                        options.prePerform.call(this, src, dest, options)
                    }
                    if (options.dry) {
                        print('Dry Run', '\n    From:    ' + src + '\n    To:      ' + dest + '\n    Options: ' + 
                            serialize(options, {pretty: true}))

                    } else if (options.perform && options.perform.call(this, src, dest, options)) {
                        /* Done if 'action' returns true */

                    //  UNUSED LEGACY
                    } else if (options.action && options.action.call(this, src, dest, options)) {
                        /* Done if 'action' returns true */

                    } else if (operation == 'move') {
                        src.rename(dest)

                    } else if (operation == 'append') {
                        if (options.divider) {
                            if (options.divider == true) {
                                contents.push('\n\n/********* Start of file ' + src + ' ************/\n\n')
                            } else {
                                contents.push(expand(options.divider, item))
                            }
                        }
                        let data = src.readString()

                        if (options.filter) {
                            data = data.replace(options.filter, '')
                        }
                        contents.push(data)

                    } else if (operation == 'copy') {
                        if (dest.endsWith(sep)) {
                            dest.makeDir()
                        } else {
                            dest.dirname.makeDir()
                        }
                        if (src.isDir) {
                            dest.makeDir()
                        } else {
                            try {
                                src.copy(dest, options)
                            } catch (e) {
                                if (options.active && Config.OS == 'windows' && dest.exists && !dest.isDir) {
                                    let active = dest.replaceExt('old')
                                    active.remove()
                                    try { dest.rename(active) } catch {}
                                }
                                src.copy(dest, options)
                            }
                        }
                    }
                }
                if (!options.perform) {
                    if (operation == 'append') {
                        /* Loop just once over the post processing */
                        commands = commands.slice(0, 1)
                        if (options.footer) {
                            contents.push(expand(options.footer, options))
                        }
                        to.write(contents.join('\n'))
                        to.setAttributes(options)
                    }
                    /*
                        Post processing: patch, strip, compress, symlink
                     */
                    for each (item in commands) {
                        let src = this.join(item.from)
                        let dest = item.to
                        let att = dest.attributes
                        if (options.patch) {
                            dest.write(expand(dest.readString(), item))
                        }
                        if (options.strip) {
                            Cmd.run('strip', dest)
                        }
                        if (options.compress) {
                            let zname = Path(dest.name + '.gz')
                            zname.remove()
                            Zlib.compress(dest.name, zname)
                            if (!options.keep) {
                                dest.remove()
                                item.dest = zname
                            }
                        }
                        if (options.symlink && Config.OS != 'windows') {
                            let symlink = Path(options.symlink)
                            if (symlink.name.endsWith(sep) || symlink.isDir) {
                                symlink.makeDir(options)
                                symlink = symlink.join(dest.basename)
                            }
                            dest.relativeTo(symlink.dirname).link(symlink)
                            item.to = symlink
                        }
                        if (att && att.permissions) {
                            dest.setAttributes({permissions: att.permissions})
                        }
                        if (options.postPerform) {
                            options.postPerform.call(this, src, dest, options)
                        }
                    }
                }
            }
            return commands.length
        }

        /**
            Trim the requested number of path components from the start or end of the path
            @param count Number of directory components to remove. If negative, trim from the end.
            @return Trimmed path
         */
        function trimComponents(count: Number): Path
            components.slice(count).join(separator)

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the end of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimEnd(pat: String): Path {
            pat = Path(pat).portable
            if (name.endsWith(pat)) {
                loc = name.lastIndexOf(pat)
                if (loc >= 0) {
                    return new Path(name.slice(0, loc))
                }
            }
            return this
        }

        /**
            Trim a file extension from a path
            @return a Path with no extension
         */
        native function trimExt(): Path

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimStart(pat: String): Path {
            if (name.startsWith(pat)) {
                return new Path(name.slice(pat.length))
            }
            return this
        }

        /**
            Reduce the size of the file by truncating it. 
            @param size The new size of the file. If the truncate argument is greater than or equal to the 
                current file size nothing happens.
            @throws IOError if the truncate failed.
         */
        native function truncate(size: Number): Void

        /**
            An equivalent normalized Windows path equivalent for the current path.
         */
        native function get windows(): Path

        //  TODO rename? - bit confusing 'write'. This really does a 'save'
        //  TODO - rename to writecContents or save
        //  TODO - last arg should be permissions?
        /**
            Write the file contents. This method opens the file, writes the contents and closes the file.
            @param args The data to write to the file. Data is serialized in before writing. Note that numbers will not 
                be written in a cross platform manner. If that is required, use the BinaryStream class to write Numbers.
            @throws IOError if the data cannot be written
         */
        function write(...args): Void {
            var file: File = new File(this, { mode: 'w', permissions: 0644 })
            try {
                for each (item in args) {
                    if (item is String) {
                        file.write(item)
                    } else {
                        file.write(serialize(item))
                    }
                }
            } 
            catch (es) {
                throw new IOError('Cannot write to file')
            }
            finally {
                file.close()
            }
        }

        /** 
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            @returns a new Path object for the temp file.
            @hide
            @deprecated 1.0.0
         */
        function makeTemp(): Path
            temp()

        /**
            Return a compact representation of the path
            This returns a relative path if the path is under the given home directory. Otherwise it returns an 
            absolute path.
            @param home Absolute path to a base home directory. Defaults to the current directory.
            @return an equivalent Path
            @hide
         */
        function compact(home: Path = App.dir): Path {
            let p = absolute
            return p.startsWith(home) ? p.relativeTo(home) : p
        }
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
