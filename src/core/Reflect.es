/*
    Reflect.es - Reflection API and class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Simple reflection class
        @spec ejs
        @stability evolving 
        @deprecated 2.0.0
     */
    # Config.Legacy
    final class Reflect {

        private var obj: Object

        use default namespace public

        /**
            Create a new reflection object.
            @param o to reflect upon
            @deprecated 2.0.0
         */
        function Reflect(o: *) {
            obj = o
        }

        /**
            The base class of the object being examined. If the object is a type, this is the super class of the type.
            @deprecated 2.0.0
         */
        function get base(): Type
            Object.getBaseType(obj)

        /**
            Test if the object is a type object
            @return True if the object is a type object
            @deprecated 2.0.0
         */
        function get isType(): Boolean
            Object.isType(obj)

        /**
            Test if the object is a prototype object
            @return True if the object is a prototype object
            @deprecated 2.0.0
         */
        function get isPrototype(): Boolean
            Object.isPrototype(obj)

        /**
            The type of the object. If the object is an instance, this is the class type object. If the object is a
            type, this value is "Type".
         */
        function get type(): Type
            Object.getType(obj)

        /**
            The prototype of the object
            @deprecated 2.0.0
         */
        function get proto(): Object
            Object.getOwnPrototypeOf(obj)

        /**
            The name of the object if it is a type object. Otherwise empty.
            @deprecated 2.0.0
         */
        function get name(): String {
            if (obj is Type) {
                return Object.getName(obj)
            }
            return null
        }

    }

/*
    FUTURE reflection proposal
 
    final class Reflect {
        native function Reflect(o: Object)

        //  MOB -- rethink
        function getInfo() {
        }
    }

    enumerable class FieldInfo {
        var name: Name
        var type: Type
        var enumerable: Boolean
        var configurable: Boolean
        var writable: Boolean
        var getter: Boolean
        var setter: Boolean

        //  MOB -- bit ugly
        var isFunction: Boolean
        var isPrototype: Boolean
        var isType: Boolean
        var isFrame: Boolean
        var isBuiltin: Boolean
        var isDynamic: Boolean
    }

    enumerable class TypeInfo {
        var name: Name
        function get superTypes: Iterator
        function get prototypes: Iterator
        function get implements: Iterator
        function get instanceMembers: Iterator
        function get staticMembers: Iterator
        function get constructor: Function

        var isDynamic: Boolean
        var isFinal: Boolean
        var isFinal: Boolean

        function isSubtypeOf(t: Type): Boolean
        function canConvertTo(t: Type): Boolean
    }

    enumerable class ParameterInfo {
        var name: Name
        var type: Type
        var isRest: Boolean
        var defaultValue: Object
    }

    enumerable class FunctionInfo {
        var parameters: Array   //  of Parameters
        var returnType: Type
        var boundThis: Object
        var boundArgs: Array
        var isMethod: Boolean
        var isInitializer: Boolean
    }
    
    enumerable class ModuleInfo {
    }
*/
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
    vim: sw=8 ts=8 expandtab

    @end
 */
