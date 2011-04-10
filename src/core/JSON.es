/*
    JSON.es -- JSON class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Encoding and decoding to JavaScript Object Notation strings (JSON). 
        This class supports the JSON data exchange format as described by: 
        RFC 4627 at (http://www.ietf.org/rfc/rfc4627.txt).
        @stability evolving
     */
    final class JSON {

        use default namespace public

        /**
            Parse a string JSON representation and return an object equivalent
            @param data JSON string data to parse
            @param filter The optional filter parameter is a function that can filter and transform the results. It 
                receives each of the keys and values, and its return value is used instead of the original value. If 
                it returns what it received, then the structure is not modified. If it returns undefined then the 
                member is deleted.  The filter function is invoked with the following signature:

                function filter(key: String, value: Object): Object

                NOTE: the filter function is not yet implemented. 

            @return An object representing the JSON string.
         */
        static function parse(data: String, filter: Function? = null): Object
            deserialize(data, { filter: filter })

        /**
            Convert an object into a string JSON representation
            @param obj Object to stringify
            @param replacer an optional function that determines how object values are stringified for objects without a 
                toJSON method. The replace has the following signature:

                function replacer(key: String, value: String): String
            @param indent optional parameter for the level of indentation of nested structures. If omitted, 
                the text will be packed without whitespace. If a number, it specifies the number of spaces 
                to indent at each level. If a string, it contains the characters used to indent at each level.
                NOTE: The indent argument is not yet implemented.
            @return A JSON string representing the object
         */
        static function stringify(obj: Object, replacer: Object? = null, indent: Object? = null): String
            serialize(obj, {replacer: replacer, indent: indent})
    }


    /** 
        Convert a string into an object. This will parse a string that has been encoded via serialize. It may contain
        nested objects and arrays. This is a static method.
        @param str The string containing the object encoding.
        @return The fully constructed object or undefined if it could not be reconstructed.
        @throws IOError If the object could not be reconstructed from the string.
        @spec ejs
     */
    native function deserialize(str: String): Object

    //  MOB -- change pretty to format: "pretty" | "compact"
    //  MOB - change to includeBases (deprecated baseClasses)
    /** 
        Encode an object as a string. This function returns a literal string for the object and all its properties. 
        If $maxDepth is sufficiently large (or zero for infinite depth), each property will be processed recursively 
        until all properties are rendered.  NOTE: the maxDepth, all and base properties are not yet supported.
        @param obj Object to serialize. If options is null, each option takes the defaults described.
        @param options Serialization options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option indent Number|String Indentation of nested structures. If omitted, the result is packed without any
            whitespace. If a number, it represents the number of spaces to indent at each level. If a string, it is
            contains the characters to indent. If the $pretty argument is true, it overrides indent.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
        @option namespaces Boolean determining if properties will also display their namespace attributes.
            Default is false.
        @option pretty Boolean determining if a human readable output is used with new lines after each property. 
            Default is false.
        @option replacer an optional parameter that determines how object values are stringified for objects without a 
            toJSON method.  The replace has the following signature:

                function replacer(key: String, value: String): String
        @return This function returns a string containing an object literal that can be used to reinstantiate an object.
        @throws TypeError If the object could not be converted to a string.
        @spec ejs
     */ 
    native function serialize(obj: Object, options: Object? = null): String
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
