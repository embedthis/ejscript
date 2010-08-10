/*
    Uri.es -- Uri parsing and management class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

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
            @param uri A string or object hash that describes the URI. The $uri specify a complete absolute URI string or
            it may specify a partial URI where missing elements take the normal defaults. The $uri argument may also be 
            an object hash with the following properties.
            @option scheme String URI protocol scheme (http or https)
            @option host String URI host name or IP address.
            @option port Number TCP/IP port number for communications
            @option path String URI path 
            @option query String URI query parameters. Does not include "?"
            @option reference String URI path reference. Does not include "#"
         */
        native function Uri(uri: Object)

        /** 
            The base of portion of the URI. The base portion is the trailing portion of the path without any 
                directory elements.
         */
        native function get basename(): Uri
        
        /** 
            A completed URI including scheme, host. The URI path will be normalized and completed with default values 
            for missing components. 
         */
        native function get complete(): Uri

        /** 
            Break a URI into its components by converting the URI to an absolute URI and breaking into components.
            The components are: scheme, host, port, path, reference and query.
            @return an object hash defining the following fields:
            @option scheme String URI protocol scheme (http or https)
            @option host String URI host name or IP address.
            @option port Number TCP/IP port number for communications
            @option path String URI path 
            @option query String URI query parameters. Does not include "?"
            @option reference String URI path reference. Does not include "#"
         */
        native function get components(): Object 

        /** 
            Decode a URI encoded string using www-url encoding. See encode for details about the character mappings.
            @param str string to decode
            @returns a decoded string
         */
        native static function decode(str: String): String

        /** 
            Decode a URI encoded component using www-url encoding. 
            See encodeComponent for details about the character mappings.
            @param str string to decode
            @returns a decoded string
         */
        native static function decodeComponent(str: String): String

        /** 
            The directory portion of a URI path. The directory portion is the leading portion including all directory 
            elements of the URI path excluding the base name. On some systems, it will include a drive specifier.
         */
        native function get dirname(): Uri

        /*
            URI encoding notes: (RFC3986). See http://labs.apache.org/webarch/uri/rfc/rfc3986.html.
            Reserved characters (and should be encoded):   : / ? # [ ] @    ! $ & ' ( ) * + , ; =
            Unreserved characters (and must not be encoded): Alpha Digits - . _ ~

            NOTE: ! , ( ) * do not yet have a formalized URI delimiting role.

            encodeComponent preserves:  ! * ' ( )
            encode preserves:           ! * ' ( ) # ; , / ? : @ 

            NOTE: encodeComponent is encoding [] which is hard for IPv6
            TODO:
                Don't encode [] for IPv6
                Encode ! ' ( ) *
         */

        /** 
            Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
            The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", 
            "*", "'", "(", ")", "#", ";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocdeURI does not encode
            "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURI routine does not. It does not
            encode "[" and "]" which may be used in IPv6 numeric hostnames.
            @param str string to encode
            @returns an encoded string
         */
        native static function encode(str: String): String

        /** 
            Encode a URI component suitable for the "application/x-www-form-urlencoded" mime type. This replaces 
            special characters with encoded alternative sequence. The encode call replaces all characters 
            except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", "'", "(", ")". It also maps space to "+".
            Compared with the $encode call, encodeComponent additionally encodes: "#", ";", ",", "/", "?", ":", "@", 
            "&", "=", "+", "$".  Note that this call encodes "=" and "&" which are used to separate and delimit 
            data form name/value pairs.
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURIComponent routine does not.
            @note See http://labs.apache.org/webarch/uri/rfc/rfc3986.html for details.
            @param str string to encode
            @returns an encoded string
         */
        native static function encodeComponent(str: String): String

        /** 
            Return true if the URI path ends with the given suffix
            @param suffix URI or String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: Object): Boolean
            path.toString().endsWith(suffix.toString())

        /** 
            The URI extension portion of the path. Set to a String containing the URI extension without the "." or set
            to null if there is no extension.
         */
        native function get extension(): String
        native function set extension(value: String): Void

        function get filename(): Path
            Path(path.slice(1))

        /** 
            Does the URI has an explicit extension
         */
        native function get hasExtension(): Boolean 

        /** 
            Does the URI have an explicit host component. For example: "www.example.com"
         */
        native function get hasHost(): Boolean 

        /** 
            Does the URI have an explicit port number.
         */
        native function get hasPort(): Boolean 

        /** 
            Does the URI have an explicit query component
         */
        native function get hasQuery(): Boolean 

        /** 
            Does the URI have an explicit reference component
         */
        native function get hasReference(): Boolean 

        /** 
            Does the URI have an explicit scheme (protocol) specifier. For example: "http://"
         */
        native function get hasScheme(): Boolean 

        /** 
            The host portion of the URI. Set to null if there is no host component.
         */
        native function get host(): String
        native function set host(value: String): Void

        /** 
            Is the URI is absolute. Set to true if the URI is an absolute path with the path component beginning with "/"
         */
        native function get isAbsolute(): Boolean

        /** 
            Is the URI is a directory URI. Set to true if the URI ends with "/". NOTE: this only tests the URI and 
            not any physical resource associated with the URI.
         */
        native function get isDir(): Boolean

        /** 
            Is the URI is a regular resource and not a directory. Set to true if the URI does not end with "/". 
            NOTE: this only tests the URI and not any physical resource associated with the URI.
         */
        function get isRegular(): Boolean
            isDir == false

        /** 
            Is if the URI is relative. Set to true if the URI's path component does not begin with "/"
         */
        function get isRelative(): Boolean
            isAbsolute == false

        /** 
            Join URIs. If a URI is absolute, replace the join with it and continue. If a URI is
            relative, replace the basename portion of the existing URI with the next joining uri and continue. For 
            example:  Uri("/admin/login").join("logout") will replace "login" with "logout" whereas 
            Uri("/admin/").join("login") will append login.
            @return A new joined URI.
         */
        native function join(...other): Uri

        /** 
            Join an extension to a URI. If the basename of the URI already has an extension, this call does nothing.
            @return A URI with an extension.
         */
        native function joinExt(ext: String): Uri

        /** 
            The mime type of the URI. This is set to a mime type string by examining the URI extension. Set to null if
            the URI has no extension.
         */
        native function get mimeType(): String

        /** 
            Normalized URI by removing all redundant and invalid URI components. Set to a URI with "segment/.." 
            and "./" components removed. The value will not be converted to an absolute URI nor will it map character case.
         */
        native function get normalize(): Uri

        /** 
            The URI path portion after the hostname
         */
        native function get path(): String
        native function set path(value: String): Void

//  MOB -- inconsistent - should this return null?
        /** 
            The port number of the URI. Set ot 80 if the URI does not have an explicit port.
         */
        native function get port(): Number
        native function set port(value: Number): Void

        /** 
            The URI protocol scheme. Set to "http" by default.
         */
        native function get scheme(): String
        native function set scheme(value: String): Void

//  MOB -- are all these null or some other default values?
        /** 
            The URI query string. The query string is the fragment after a "?" character in the URI.
         */
        native function get query(): String
        native function set query(value: String): Void

        /** 
            The URI reference portion. The reference portion is sometimes called the "anchor" and is the the fragment 
            after a "#" character in the URI.
         */
        native function get reference(): String
        native function set reference(value: String): Void

        /** 
            Create a URI with a releative path from the current URI to a given URI. This call computes the relative
            path from this URI to the $target URI argument.
            @param target Uri Target URI to locate.
            @return a new URI object for the target URI
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
            Replace the extension and return a new URI.
            @return A path with extension.
         */
        native function replaceExt(ext: String): Uri

        /** 
            Compare two URIs test if they represent the same resource
            @param other Other URI to compare with
            @param exact If exact is true, then the query and reference portions must match
            @return True if the URIs represent the same underlying resource
         */
        native function same(other: Object, exact: Boolean = false): Boolean

        /** 
            Return true if the URI path starts with the given prefix. This skips the scheme, host and port portions
            and examines the URI path only.
            @param prefix URI or String prefix to compare with the URI.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: Object): Boolean
            path.toString().startsWith(prefix.toString()) 

        /** 
            Convert the URI to a JSON string. 
            @return a JSON string representing the URI.
         */
        override function toJSON(): String
            JSON.stringify(this.toString())

        /** 
            Convert the URI to a string. The format of the string will depend on the defined $representation format.
            @return a string representing the URI.
         */
        native override function toString(): String

        /** 
            Trim a pattern from the end of the URI path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a new URI containing the trimmed URI
            TODO - should support reg expressions
         */
        function trimEnd(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimEnd(pat)
            return u
        }

        /** 
            Trim the extension portion off the URI path
            @return a URI with no extension
         */
        native function trimExt(): Uri

        /** 
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a URI containing the trimmed path name
            TODO - should support reg expressions
         */
        function trimStart(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimStart(pat)
            return u
        }

        /** 
            The full URI as a string.
         */
        native function get uri(): String
        native function set uri(value: String): Void
    }

    /** 
        Decode an encoded URI using www-url encoding
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURI(str: String): String

    /** 
        Decode an encoded URI component.
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURIComponent(str: String): String

    /** 
        Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")", "#",";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocdeURI does not encode
        "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
        @see Uri.encode for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURI(str: String): String

    /** 
        Encode a URI component using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")". Note that this call encodes "=" and "&" which are often used in URL query name/key pairs.
        @see Uri.encodeComponent for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURIComponent(str: String): String

    /** 
        Encode objects using using www-url encoding. Each object is encoded as a "key=value" pair. Each pair is separated
        by a "&" character. 
        @param items Strings to encode
        @returns an encoded string
     */
    function encodeObjects(...items) {
        let result = ""
        for each (item in items) {
            for (let [key, value] in item) {
                if (result) result += "&"
                result += encodeURIComponent(key) + "=" + encodeURICompnent(value)
            }
        }
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
