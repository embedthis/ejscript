/*
    Function.es -- Function class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Function type is used to represent functions, function expressions, closures and class methods. It contains a 
        reference to the code to execute, the execution scope and possibly a bound "this" reference.
        @stability evolving
     */
    native class Function {

        use default namespace public

        /**
            Create a function from the supplied formal parameter names and function body expression.
            @param args The parameters and body are suplied as discrete parameters to Function(), i.e. 
                not as an array of args.
            Function ([args, ...], body)
         */
        native function Function(...args)
/*
            let body = args.pop()
            let code = "function(" + args.join(",") + ") {\n" + body + "\n}"
            print("CODE " + code)
            eval(code)
        }
*/

        /** 
            Invoke the function on another object.
            @param thisObject The object to set as the "this" object when the function is called.
            @param args Array of actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function apply(thisObject: Object, args: Array): Object 

        /** 
            Invoke the function on another object. This function takes the "this" parameter and then a variable 
                number of actual parameters to pass to the function.
            @param thisObject The object to set as the "this" object when the function is called. 
            @param args Actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function call(thisObject: Object, ...args): Object 

        //    MOB -- this could go into Reflect
        /** 
            The bound object representing the "this" object. Functions carry both a lexical scoping and the owning 
            "this" object. Set to "null" if "this" is not defined for the function.
         */
        native function get boundThis(): Object

        //  MOB -- ES5 usage is function bind(obj, ...args)
        /** 
            Bind the value of "this" for the function. This can set the value of "this" for the function. If
            $overwrite is false, it will only define the value of "this" if it is not already defined.
            @param thisObj Value of "this" to define
            @param overwrite If true, overwrite the existing stored value of "this"
         */
        native function bind(thisObj: Object, overwrite: Boolean = true): Void

//  MOB -- ECMA: this is a var on all functions and not a getter on the prototype
        //  Number of arguments expected by the function
        native function get length(): Number

        //  MOB -- DOC
        /** @hide */
        native function setScope(scope: Object): Void
    }

    //  MOB -- remove
    /** @hide */
    native function makeGetter(fn: Function): Function

    //  MOB -- move into Object   Object.
    /** @hide */
    native function clearBoundThis(fn: Function): Function
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
