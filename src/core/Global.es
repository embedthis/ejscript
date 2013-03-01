/*
    Global.es -- Global variables, namespaces and functions.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB - fix order

    /**
        Constant set to true in all Ejscript interpreters
        @spec ejs
      */
    public var EJSCRIPT: Boolean = true

    /** 
        The "ejs" namespace used for the core library
        @spec ejs
     */
    public namespace ejs

    /** 
        The public namespace used to make entities visible accross modules.
        @hide
     */
    public namespace public

    //  MOB - is this used as a global?
    /** 
        The internal namespace used to make entities visible within a single module only.
        @spec ejs
     */
    public namespace internal

    //  MOB -- remove iterator namespace
    /** 
        The iterator namespace used to define iterators.
        @spec ejs
     */
    public namespace iterator

    /** 
        Alias for the Boolean type
       @spec ejs
     */
    const boolean: Type

    /** 
        Alias for the Number type
     */
    const double: Type

    /** 
       Alias for the Number type
       @spec ejs
     */
    const num: Type

    /** 
        Alias for the String type
     */
    const string: Type

    /**
        Empty string value
     */
    const empty: String

    /** 
        Boolean false value.
     */
    const false: Boolean

    /** 
        Global variable space reference. The global variable references an object which is the global variable space. 
        This is useful when guaranteed access to a global variable is required. e.g. global.someName.
        @spec ejs
     */
    var global: Object

    /** 
        Null value. The null value is returned when testing a nullable variable that has not yet had a 
        value assigned or one that has had null explicitly assigned.
     */
    const null: Null

    /** 
        Infinity value.
     */
    const Infinity: Number

    /** 
        Negative infinity value.
     */
    const NegativeInfinity: Number

    /** 
        Invalid numeric value. If the numeric type is set to an integral type, the value is zero.
     */
    const NaN: Number

    /** 
        True value
     */
    const true: Boolean

    /** 
        Undefined variable value. The undefined value is returned when testing
        for a property that has not been defined. 
     */
    const undefined: Void

    /** 
        Void type value. 
        This is an alias for Void.
        @spec ejs
     */
    const void: Type

    /* Internal slot reservations */
    internal const commaProt: String
    internal const one: Number
    internal const zero: Number
    internal const length: String
    internal const minusOne: Number
    internal const emptySpace: Namespace
    internal const max: Number
    internal const min: Number
    internal const nop: Function

    /** 
        Assert a condition is true. This call tests if a condition is true by testing to see if the supplied 
        expression is true. If the expression is false, the interpreter will throw an exception.
        @param condition JavaScript expression evaluating or castable to a Boolean result.
        @throws AssertError if the condition is false.
        @spec ejs
     */
    native function assert(condition: Boolean): Void

    //  MOB - is this used?
    /** 
        Replace the base type of a type with an exact clone. 
        @param klass Class in which to replace the base class.
        @spec ejs
        @hide
     */
    native function cloneBase(klass: Type): Void

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
        Blend the contents of one object into another. This routine copies the properites of one object into another.
        If a property is an object reference and the "deep" option is set, the object is cloned using a deep clone. 
        Otherwise the object reference is copied.
        If the overwrite option is false, the property will only be copied if it does not already exist in the destination.
        @param dest Destination object
        @param src Source object
        @param options Control options
        @option combine Boolean. If true, then support key prefixes "+", "=", "-", "?" to add, assign and subtract 
            and conditionally assign key values.
            When adding string properties, values will be appended using a space separator. Extra spaces will not 
            be removed on subtraction.
            Arrays with string values may also be combined using the key prefixes. 
        @option overwrite Boolean. If true, then overwrite existing properties in the destination object. Defaults to true.
        @option deep Boolean. If true, then recursively copy the properties of any objects referenced in the source object.
            Otherwise, the copy is only one level deep. Defaults to true.
        @option functions Boolean. If true, then copy functions. Defaults to false.
        @option subclasses Boolean. If true, then copy subclass properties. Defaults to true.
        @option trace Boolean. If true, then trace to the App.log the copied property names.
        @returns An the destination object
        @spec ejs
        @example blend(dest, src, { overwrite: true, deep: true, functions: false, subclasses: true })
     */
    native function blend(dest: Object, src: Object, options = null): Object

    // TODO - should cache be a Path
    /** 
        Evaluate a script. Not present in ejsvm.
        @param script Script string to evaluate
        @returns the the script expression value.
     */
    native function eval(script: String, cache: String? = null): Object

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
        @param options Option parameters
        @options cache String or Path name to save the compiled script as.
        @options reload Boolean If true, reload the module if already loaded. Defaults to true.
        @returns The value of the last expression in the loaded module
     */
    native function load(file: String, options: Object? = null): Object

    /** 
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method. 
        @param args Variables to print
        @spec ejs
     */
    native function print(...args): void

    /** 
        Print the arguments to the standard output using the supplied format template. This call evaluates the arguments, 
        converts the result to strings and invokes String.format to format the args. The result is then printed to the
        standard output. Arguments are converted to strings by calling their toString method. 
        @param fmt Format specifier
        @param args Variables to print
        @spec ejs
     */
    function printf(fmt: String, ...args): Void
        App.outputStream.write(fmt.format(args))

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

    /** @hide */
    function printHash(name: String, o: Object): Void
        print("%20s %X" % [name, hashcode(o)])

    /**  @hide TODO - doc */
    function instanceOf(obj: Object, target: Object): Boolean
        obj is target

    /** @hide */
    native function base64(str: String): String
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
