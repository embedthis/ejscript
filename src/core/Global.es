/*
    Global.es -- Global variables, namespaces and functions.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Constant set to true in all Ejscript interpreters
      */
    public var EJSCRIPT: Boolean = true

    //  TODO - remove
    /** @hide */
    public var ECMA: Boolean = false

//MOB
public namespace ejs
    /** 
        The public namespace used to make entities visible accross modules.
     */
    public namespace public

    /** 
        The internal namespace used to make entities visible within a single module only.
     */
    public namespace internal

    /** 
        The iterator namespace used to defined iterators.
     */
    public namespace iterator

//  MOB -- is this needed
    /** 
        The CONFIG namespace used to defined conditional compilation directives.
     */
    public namespace CONFIG

    //  MOB -- is this needed?
    use namespace iterator

    //  TODO - refactor and reduce these
   
//  MOB -- rationalize all this
    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
     */  
    const TODO: Boolean = false

    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
     */  
    const FUTURE: Boolean = false

    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
     */  
    const ASC: Boolean = false

    /** 
        Conditional compilation constant. Used to enable the compilation of elements only for creating the API documentation.
     */  
    const DOC_ONLY: Boolean = false

    /** 
        Conditional compilation constant. Used to deprecate elements.
     */  
    const DEPRECATED: Boolean = false

    //  TODO - remove. Should be using Config.RegularExpressions
   
    /** 
        Alias for the Boolean type
     */
    native const boolean: Type

//  TODO - rationalize these aliases
    /** 
        Alias for the Number type
     */
    native const double: Type

//  TODO - rationalize these aliases
    /** 
        Alias for the Number type
        @spec ejs
     */
    native const num: Type

    /** 
        Alias for the String type
     */
    native const string: Type

    /** 
        Boolean false value.
     */
    native const false: Boolean

    /** 
        Global variable space reference. The global variable references an object which is the global variable space. 
        This is useful when guaranteed access to a global variable is required. e.g. global.someName.
     */
    native var global: Object

    /** 
        Null value. The null value is returned when testing a nullable variable that has not yet had a 
        value assigned or one that has had null explicitly assigned.
     */
    native const null: Null

    /** 
        Infinity value.
     */
    native const Infinity: Number

    /** 
        Negative infinity value.
     */
    native const NegativeInfinity: Number

    /** 
        Invalid numeric value. If the numeric type is set to an integral type, the value is zero.
     */
    native const NaN: Number

    /** 
        True value
     */
    native const true: Boolean

    /** 
        Undefined variable value. The undefined value is returned when testing
        for a property that has not been defined. 
     */
    native const undefined: Void

    /** 
        Void type value. 
        This is an alias for Void.
        @spec ejs
     */
    native const void: Type

    /** 
        Assert a condition is true. This call tests if a condition is true by testing to see if the supplied 
        expression is true. If the expression is false, the interpreter will throw an exception.
        @param condition JavaScript expression evaluating or castable to a Boolean result.
        @return True if the condition is.
        @spec ejs
     */
    native function assert(condition: Boolean): Boolean

    /** 
        Convenient way to trap to the debugger
     */
    native function breakpoint(): Void

    /** 
        Replace the base type of a type with an exact clone. 
        @param klass Class in which to replace the base class.
        @spec ejs
     */
    native function cloneBase(klass: Type): Void

    /**
        Reverse www-url encoding on a string
        @param str URL encoded string
        @returns a decoded string
     */
    native function decodeURI(str: String): String

    //  TODO - consider renaming to debug()
    /** 
        Dump the contents of objects. Used for debugging, this routine serializes the objects and prints to the standard
        output.
        @param args Variable number of arguments of any type
        @hide
     */
    function dump(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true}))
        }
    }

    /** @hide */
    function dumpAll(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true, hidden: true, baseClasses: true}))
        }
    }

    /** @hide */
    function dumpDef(...args): Void {
        for each (var o: Object in args) {
            for each (var key: Object in Object.getOwnPropertyNames(o)) {
                print(key + ": " + serialize(Object.getOwnPropertyDescriptor(o, key), {pretty: true, depth: 1}))
            }
        }
    }


    /**
        Encode a string using  www-url encoding
        @param str URL encoded string
        @returns an encoded string
     */
    native function encodeURI(str: String): String

    /** 
        HTML escape a string. This quotes characters which would otherwise be interpreted as HTML directives.
        This function is prototype and may be renamed in a future release.
        @param str String to html escape
        @returns a HTML escaped string
        @spec ejs
     */
    native function escape(str: String): String

    //  TODO - move to Crypt
    /** 
        Computed an MD5 sum of a string
        This function is prototype and may be renamed in a future release.
        @param str String on which to compute a checksum
        @returns An MD5 checksum
        @spec ejs
     */
    native function md5(str: String): String

    /** 
        Blend one object into another. This is useful for inheriting and optionally overwriting option hashes (among other
        things). The merge is done at the primitive property level and it does a deep clone of the source. If overwrite 
        is true, the property is replaced. If overwrite is false, the property will be added if it does not already exist
        @param dest Destination object
        @param src Source object
        @param overwrite Boolean. If true, then overwrite existing properties in the destination object.
        @returns An the destination object
        @spec ejs
        @hide
     */
    native function blend(dest: Object, src: Object, overwrite: Boolean = true): Object

     // TODO - should cache be a Path
    /** 
        Evaluate a script. Not present in ejsvm.
        @param script Script string to evaluate
        @returns the the script expression value.
     */
    native function eval(script: String, cache: String? = null): Object

    //  TODO - move this to System/App/Debug and use "platform" (internal use only) namespace
    /**
        Format the current call stack. Used for debugging and when creating exception objects.
        @spec ejs
     */
    native function formatStack(): String

    /** 
        Get the object's Unique hash id. All objects have a unique object hash. 
        @return This property accessor returns a long containing the object's unique hash identifier. 
     */ 
    native function hashcode(o: Object): Number

    /**
        Evaluate an argument to determine if it is a number.
        @param arg to evaluate.
        @return True if the argument is a number
     */
    function isNaN(arg: Number): Boolean
        arg.isNaN

    /**
        Evaluate an argument to determine if it is a finite number.
        @param arg to evaluate.
        @return True if the argument is a finite number
     */
    function isFinite(arg: Number): Boolean
        arg.isFinite

    //  TODO - should file and cache be paths?
    /** 
        Load a script or module
        @param file path name to load. File will be interpreted relative to EJSPATH if it is not found as an absolute or
            relative file name.
        @param cache path name to save the compiled script as.
        @returns The value of the last expression in the loaded module
     */
    native function load(file: String, cache: String? = null): Object

    /** 
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method. This method invokes $output as its implementation. 
        @param args Variables to print
        @spec ejs
     */
    native function print(...args): void

    /** 
        Parse a string and convert to a primitive type
        @param str String to parse
        @param preferredType Preferred type to use if the input can be parsed multiple ways.
        @return Parsed object
     */
    native function parse(str: String, preferredType: Type? = null): Object

    /** 
        Parse a string as a floating point number 
        @param str String to parse
        @return A parsed number
     */
    function parseFloat(str: String): Number
        parse(str, Number)

    /** 
        Parse a string and convert to an integral number using the specified radix.
        @param str String to parse
        @param radix Base radix to use when parsing the number.
        @return A parsed number
     */
    native function parseInt(str: String, radix: Number = 10): Number

    /* TODO - remove
        Determine the type of a variable. 
        @param o Variable to examine.
        @return Returns a string containing the arguments type. Possible types are:
            @li $undefined "undefined"
            @li $Object "object"
            @li $Boolean "boolean"
            @li $Function "function"
            @li Number "number"
            @li String "string"
        @remarks Note that lower case names are returned for class names.
        @spec ejs
    native function typeof(o: Object): String
     */

    /** @hide  TODO - temp only */
    function printHash(name: String, o: Object): Void
        print("%20s %X" % [name, hashcode(o)])

    /**  @hide TODO - doc */
    function instanceOf(obj: Object, target: Object): Boolean
        obj is target

    /**  
        DEPREACTED - Use App.stderr.write() and App.stderr.writeLine()
        Write to the standard error. This call writes the arguments to the standard error with a new line appended. 
        It evaluates the arguments, converts the result to strings and prints the result to the standard error. 
        Arguments are converted to strings by calling their toSource method.
        @param args Data to write
        @spec ejs
        @hide
     */
    native function error(...args): void

    /**
        DEPREACTED
        Read from the standard input. This call reads a line of input from the standard input
        @return A string containing the input. Returns null on EOF.
        @hide
     */
    native function input(): String

    /**  
        DEPREACATED - Use print(), App.stdout.write() and App.stdout.writeLine()
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method.
        @param args Variables to print
        @spec ejs
        @hide
     */
    native function output(...args): void
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
