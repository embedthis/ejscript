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
    class Function {

        use default namespace public

        /**
            Create a function from the supplied formal parameter names and function body expression.
            @param args The parameters and body are suplied as discrete parameters to Function(), i.e. 
                not as an array of args.
            Function ([args, ...], body)
         */
        native function Function(...args)
/*
    FUTURE -- todo
            let body = args.pop()
            let code = "function(" + args.join(",") + ") {\n" + body + "\n}"
            print("CODE " + code)
            return eval(code)
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

        //  MOB this should return a bound function
        /** 
            Bind the value of "this" for the function. This can set the value of "this" for the function. If
            $overwrite is false, it will only define the value of "this" if it is not already defined.
            Use $bound to examine the bound "this" value.
            @param thisObj Value of "this" to define
            @param args Function arguments to supply to the function. These arguments preceed any caller supplied
                arguments when the function is actually invoked.
         */
        native function bind(thisObj: Object, ...args): Void

        /** 
            The bound object representing the "this" object for the function. Will be set to null if no object is bound.
            Use $bind() to set the bound "this" object.
            @see bind
         */
        native function get bound(): Object?

        /**
            The name of the function. Function expressions do not have a name and set the name property to the empty string.
         */
        native function get name(): String

        //  Number of arguments expected by the function
        native function get length(): Number

        //  DOC
        /** @hide */
        native function setScope(scope: Object): Void
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.md distributed with 
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
