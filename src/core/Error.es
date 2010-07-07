/*
    Error.es -- Error exception classes

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        Arguments error exception class. 
        Thrown the arguments cannot be cast to the required type or in strict mode if there are too few or too 
        many arguments.
        @spec ejs
        @stability evolving
     */
    native dynamic class ArgError extends Error {
        /**
            ArgError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArgError(message: String? = null) 
    }

    /**
        Arithmetic error exception class. Thrown when the system cannot perform an arithmetic operation, 
        @spec ejs
        @stability evolving
     */
    native dynamic class ArithmeticError extends Error {
        /**
            ArithmeticError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArithmeticError(message: String? = null) 
    }

    /**
        Assertion error exception class. Thrown when the $assert method is invoked with a false value.
        @spec ejs
        @stability evolving
     */
    native dynamic class AssertError extends Error {
        /**
            AssertError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function AssertError(message: String? = null) 
    }

    /**
        Code (instruction) error exception class. Thrown when an illegal or insecure operation code is detected 
        in the instruction stream.
        @spec ejs
        @stability evolving
     */
    native dynamic class InstructionError extends Error {
        /**
            InstructionError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InstructionError(message: String? = null) 
    }

    /**
        Base class for error exception objects. Exception objects are created by the system as part of changing 
        the normal flow of execution when some error condition occurs. 

        When an exception is created and acted upon ("thrown"), the system transfers the flow of control to a 
        pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
        point at which the exception was thrown or not. It may re-throw the exception or pass control up the call stack.
        @stability evolving
     */
    native dynamic class Error {

        use default namespace public

        /** 
            Source filename of the script that created the error
         */
        function get filename(): String
            stack[0].filename

        /** 
            Source line number in the script that created the error
         */
        function get lineno(): String
            stack[0].lineno

        /**
            Supplemental error data
         */
        var data: Object

        /**
            Error message
         */
        var message: String

        private var _stack: Array

        /**
            Execution call stack. Contains the execution stack backtrace at the point the Error object was created.
            The stack is an array of stack frame records. Each record consists of the following properties: 
                {file: String, line: Number, functionName: String, code: String}
         */
        function get stack(): Array {
            if (!_stack) {
                _stack = capture()
            }
            return _stack
        }

        /** 
            Time the event was created. The Context constructor will automatically set the timestamp to the current time.  
         */
        var timestamp: Date

        /**
            Optional error code
         */
        var code: Number

        /**
            Construct a new Error object.
            @params message Message to use when defining the Error.message property. Typically a string but can be an
                object of any type.
         */
        native function Error(message: String? = null)

        /**
            Capture the stack. This call re-captures the stack and updates the stack property.
            @param uplevels Skip a given count of stack frames from the stop of the call stack.
         */
        native function capture(uplevels: Number): Array

        function formatStack(): String {
            let result = ""
            let i = 0
            for each (frame in stack) {
                result += " [%02d] %s, line %d, %s(), %s\n".format(i++, ...frame)
            }
            return result
        }
    }

    /**
        IO error exception class. Thrown when an I/O/ interruption or failure occurs, e.g. a file is not found 
        or there is an error in a communication stack.
        @spec ejs
        @stability evolving
     */
    native dynamic class IOError extends Error {
        /**
            IOError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function IOError(message: String? = null) 
    }

    /**
        Internal error exception class. Thrown when some error occurs in the virtual machine.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class InternalError extends Error {
        /**
            InternalError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InternalError(message: String? = null) 
    }

    /**
        Memory error exception class. Thrown when the system attempts to allocate memory and none is available 
        or the stack overflows.
        @spec ejs
        @stability evolving
     */
    native dynamic class MemoryError extends Error {
        /**
            MemoryError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function MemoryError(message: String? = null) 
    }

    /**
        OutOfBounds error exception class. Thrown to indicate that an attempt has been made to set or access an 
        object's property outside of the permitted set of values for that property. For example, an array has been 
        accessed with an illegal index or, in a date object, attempting to set the day of the week to greater then 7.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class OutOfBoundsError extends Error {
        /**
            OutOfBoundsError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function OutOfBoundsError(message: String? = null) 
    }

    /**
        Reference error exception class. Thrown when an invalid reference to an object is made, e.g. a method is 
        invoked on an object whose type does not define that method.
        @stability evolving
     */
    native dynamic class ReferenceError extends Error {
        /**
            ReferenceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ReferenceError(message: String? = null)
    }

    /**
        Resource error exception class. Thrown when the system cannot allocate a resource it needs to continue, 
        e.g. a native thread, process, file handle or the like.
        @spec ejs
        @stability evolving
     */
    native dynamic class ResourceError extends Error {
        /**
            ResourceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ResourceError(message: String? = null) 
    }

    /**
        Security error exception class. Thrown when an access violation occurs. Access violations include attempting 
        to write a file without having write permission or assigning permissions without being the owner of the 
        securable entity.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class SecurityError extends Error {
        /**
            SecurityError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SecurityError(message: String? = null) 
    }

    /**
        State error exception class. Thrown when an object cannot be transitioned from its current state to the 
        desired state, e.g. calling "sleep" on an interrupted thread.
        @spec ejs
        @stability evolving
     */
    native dynamic class StateError extends Error {
        /**
            StateError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function StateError(message: String? = null) 
    }

    /**
        Syntax error exception class. Thrown when the system cannot parse a character sequence for the intended 
        purpose, e.g. a regular expression containing invalid characters.
        @stability evolving
     */
    native dynamic class SyntaxError extends Error {
        /**
            SyntaxError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SyntaxError(message: String? = null) 
    }

    /**
        Type error exception class. Thrown when a type casting or creation operation fails, e.g. when an operand 
        cannot be cast to a type that allows completion of a statement or when a type cannot be found for object 
        creation or when an object cannot be instantiated given the values passed into "new".
        @stability evolving
     */
    native dynamic class TypeError extends Error {
        /**
            TypeError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function TypeError(message: String? = null) 
    }

    /**
        Uri error exception class. Thrown a Uri fails to parse.
        @stability prototype
        @hide
     */
    native dynamic class URIError extends Error {
        /**
            URIError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function URIError(message: String? = null) 
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2010-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 2010-2010. All Rights Reserved.
    
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
 *
 */
