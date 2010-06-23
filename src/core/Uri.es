/*
    Uri.es -- Uri parsing and management class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB -- should this be URI or Uri

    /**
        Uri class to manage Uris
        @stability evolving
        @spec ejs
        @stability prototype
     */
    class Uri {
        use default namespace public

        /** 
            Create and parse a Uri object. 
            @param uri A string or object hash that describes the Uri. The $uri specify a complete absolute Uri string or
            it may specify a partial Uri where missing elements take the normal defaults. The $uri argument may also be 
            an object hash with the following properties.
            @option scheme: String
            @option host: String
            @option port: Number
            @option path: String
            @option query: String
            @option reference: String
         */
        native function Uri(uri: Object)

        /** 
            The base of portion of the Uri. The base portion is the trailing portion of the path without any 
                directory elements.
         */
        native function get basename(): Uri
        
        /** 
            A completed Uri including scheme, host. The Uri path will be normalized and completed with default values 
            for missing components. 
         */
        native function get complete(): Uri

        /** 
            Break a Uri into its components by converting the Uri to an absolute Uri and breaking into components.
            The components are: scheme, host, port, path, reference and query.
            @return an object hash defining the following fields:
            @option scheme: String
            @option host: String
            @option port: Number
            @option path: String
            @option query: String
            @option reference: String
         */
        native function get components(): Object 

        /** 
            Decode a Uri encoded string
            @param str string to decode
            @returns a decoded string
         */
        native static function decode(str: String): String

        /** 
            Decode a Uri encoded component 
            @param str string to decode
            @returns a decoded string
         */
        native static function decodeComponent(str: String): String

        /** 
            The directory portion of a Uri path. The directory portion is the leading portion including all directory 
            elements of the Uri path excluding the base name. On some systems, it will include a drive specifier.
         */
        native function get dirname(): Uri

        /** 
            Encode a Uri
            @param str string to encode
            @returns an encoded string
         */
        native static function encode(str: String): String

        /** 
            Encode a Uri component
            @param str string to encode
            @returns an encoded string
         */
        native static function encodeComponent(str: String): String

        /** 
            Return true if the Uri path ends with the given suffix
            @param suffix Uri or String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: Object): Boolean
            path.toString().endsWith(suffix.toString())

        /** 
            The Uri extension portion of the path. Set to a String containing the Uri extension without the "." or set
            to null if there is no extension.
         */
        native function get extension(): String

        /** 
            Set the Uri extension portion
            @param value String containing the new extension. 
         */
        native function set extension(value: String): Void

        function get filename(): Path
            Path(path.slice(1))

        /** 
            Does the Uri has an explicit extension
         */
        native function get hasExtension(): Boolean 

        /** 
            Does the Uri have an explicit host component. For example: "www.example.com"
         */
        native function get hasHost(): Boolean 

        /** 
            Does the Uri have an explicit port number.
         */
        native function get hasPort(): Boolean 

        /** 
            Does the Uri have an explicit query component
         */
        native function get hasQuery(): Boolean 

        /** 
            Does the Uri have an explicit reference component
         */
        native function get hasReference(): Boolean 

        /** 
            Does the Uri have an explicit scheme (protocol) specifier. For example: "http://"
         */
        native function get hasScheme(): Boolean 

        /** 
            The host portion of the Uri. Set to null if there is no host component.
         */
        native function get host(): String

        /** 
            @duplicate Uri.host
            @param value A string containing the new host portion
         */
        native function set host(value: String): Void

        /** 
            Is the Uri is absolute. Set to true if the Uri is an absolute path with the path component beginning with "/"
         */
        native function get isAbsolute(): Boolean

        /** 
            Is the Uri is a directory Uri. Set to true if the Uri ends with "/". NOTE: this only tests the Uri and 
            not any physical resource associated with the Uri.
         */
        native function get isDir(): Boolean

        /** 
            Is the Uri is a regular resource and not a directory. Set to true if the Uri does not end with "/". 
            NOTE: this only tests the Uri and not any physical resource associated with the Uri.
         */
        function get isRegular(): Boolean
            isDir == false

        /** 
            Is if the Uri is relative. Set to true if the Uri's path component does not begin with "/"
         */
        function get isRelative(): Boolean
            isAbsolute == false

        /** 
            Join Uris. Joins Uris together. If a Uri is absolute, replace the join with it and continue. If a Uri is
            relative, replace the basename portion of the existing Uri with the next joining uri and continue. For 
            example:  Uri("/admin/login").join("logout") will replace "login" with "logout" whereas 
            Uri("/admin/").join("login") will append login.
            @return A joined, normalized Uri.
         */
        native function join(...other): Uri

        /** 
            Join an extension to a Uri. If the basename of the Uri already has an extension, this call does nothing.
            @return A Uri with an extension.
         */
        native function joinExt(ext: String): Uri

        /** 
            The mime type of the Uri. This is set to a mime type string by examining the Uri extension. Set to null if
            the Uri has no extension.
         */
        native function get mimeType(): String

        /** 
            Normalized Uri by removing all redundant and invalid Uri components. Set to a Uri with "segment/.." 
            and "./" components removed. The value will not be converted to an absolute Uri nor will it map character case.
         */
        native function get normalize(): Uri

        /** 
            The Uri path portion after the hostname
         */
        native function get path(): String

        /** 
            @duplicate Uri.path
            @param value String containing the new path portion
         */
        native function set path(value: String): Void

//  MOB -- inconsistent - should this return null?
        /** 
            The port number of the Uri. Set ot 80 if the Uri does not have an explicit port.
         */
        native function get port(): Number

        /** 
            @duplicate Uri.port
            @param value Number of the port
         */
        native function set port(value: Number): Void

        /** 
            The Uri protocol scheme. Set to "http" by default.
         */
        native function get scheme(): String

        /** 
            @duplicate Uri.scheme
            @param value String containing the new protocol scheme. For example, to select FTP, use "ftp"
         */
        native function set scheme(value: String): Void

//  MOB -- are all these null or some other default values?
        /** 
            The Uri query string. The query string is the fragment after a "?" character in the Uri.
         */
        native function get query(): String

        /** 
            @duplicate Uri.query
            @param value A string containing the new query string portion. 
         */
        native function set query(value: String): Void

        /** 
            The Uri reference portion. The reference portion is sometimes called the "anchor" and is the the fragment 
            after a "#" character in the Uri.
         */
        native function get reference(): String

        /** 
            @duplicate Uri.reference
            @param value A string containing the new reference string portion. 
         */
        native function set reference(value: String): Void

        /** 
            Create a Uri with a releative path from the current Uri to a given Uri. This call computes the relative
            path from this Uri to the $target Uri argument.
            @param target Uri Target Uri to locate.
            @return a new Uri object for the target Uri
         */
        function relative(target: Uri): Uri {
            let parts = this.normalize.path.toString().split("/")
            let targetParts = target.normalize.path.toString().split("/")
            if (parts.length < targetParts.length) {
                u = this.clone()
                u.path = targetParts.slice(parts.length).join("/")
                return u
            } else {
                let results = ""
                len = parts.length.min(targetParts.length)
                for (common = 0; common < len; common++) {
                    if (parts[common] != targetParts[common]) {
                        break
                    }
                }
                results = "../".times(parts.length - common - 1)
                if (targetParts.length > 1) {
                    results += targetParts.slice(common).join("/")
                }
                results = results.trimEnd("/")
                return Uri(results)
            }
        }

        /** 
            Replace the extension and return a new Uri.
            @return A path with extension.
         */
        native function replaceExt(ext: String): Uri

        /** 
            Compare two Uris test if they represent the same resource
            @param other Other Uri to compare with
            @param exact If exact is true, then the query and reference portions must match
            @return True if the Uris represent the same underlying resource
         */
        native function same(other: Object, exact: Boolean = false): Boolean

        /** 
            Return true if the Uri path starts with the given prefix. This skips the scheme, host and port portions
            and examines the Uri path only.
            @param prefix Uri or String prefix to compare with the Uri.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: Object): Boolean
            path.toString().startsWith(prefix.toString()) 

        /** 
            Convert the Uri to a JSON string. 
            @return a JSON string representing the Uri.
         */
        override function toJSON(): String
            JSON.stringify(this.toString())

        /** 
            Convert the Uri to a string. The format of the string will depend on the defined $representation format.
            @return a string representing the Uri.
         */
        native override function toString(): String

        /** 
            Trim a pattern from the end of the Uri path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a new Uri containing the trimmed Uri
            TODO - should support reg expressions
         */
        function trimEnd(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimEnd(pat)
            return u
        }

        /** 
            Trim the extension portion off the Uri path
            @return a Uri with no extension
         */
        native function trimExt(): Uri

        /** 
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a Uri containing the trimmed path name
            TODO - should support reg expressions
         */
        function trimStart(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimStart(pat)
            return u
        }

        /** 
            The full Uri as a string.
         */
        native function get uri(): String

        /** 
            Set the Uri 
            @param value String containing the new Uri
         */
        native function set uri(value: String): Void
    }

    /** 
        Decode an encoded Uri.
        @param str encoded string
        @returns a decoded string
     */
    function decodeUri(str: String): String
        Uri.decode(str)

    /** 
        Decode an encoded Uri component.
        @param str encoded string
        @returns a decoded string
     */
    function decodeUriComponent(str: String): String
        Uri.decodeUriComponent(str)

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
