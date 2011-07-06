/*
    Object.es -- Object class. Base class for all types.
    Copyright (c) All Rights Reserved. See details at the end of this file.
 */

module ejs {
    /** 
        The Object Class is the root class from which all objects are based. It provides a foundation set of functions 
        and properties which are available to all objects. It provides for: copying objects, evaluating equality of 
        objects, providing information about base classes, serialization and deserialization and iteration. 
        @stability evolving
     */
    dynamic class Object implements Iterable {

        use default namespace public

        /**
            The constructor function from which this object was created.
         */
        native function get constructor(): Function

        /** 
            Clone the object
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            A shallow clone will do 1 level deep. Deep clones are N-level deep.
            @spec ejs
         */
        native function clone(deep: Boolean = true): Object

        /** 
            Create a new object, set the object prototype and properties.
            @param prototype Prototype object
            @param properties Properties for the new object
            @return The created object
         */
        static native function create(prototype: Object, properties: Object = undefined): Object 

        /** 
            Define or redefine a property on the given object
            @param obj Object on which to define the property
            @param name Property name
            @param options Property descriptor object. The descriptor has properties for: configurable, enumerable, get, 
                namespace, set, value, and writable.
            @option configurable If true, make the property configurable. This means it can be deleted and have its
                properties modified.
            @option enumerable If true, make the property visible to for/in enumerations.
            @option get Getter function to return the property value
            @option namespace Property namespace qualifier
            @option set Setter function to update the property value
            @option value Initial property value. Can't use if a get or set property is defined.
            @option writable If true, the property value may be updated after the initial definition.
        */
        static native function defineProperty(obj: Object, name: String, options: Object): Void

        /**
            Define a set of properties on the given object
            @param obj Object on which to define the property
            @param properties Hash of property descriptors
         */
        static function defineProperties(obj: Object, properties: Object): Void {
            for (p in properties) {
                defineProperty(obj, p, properties[p])
            }
        }

        /** 
            Freeze an object. This freezes the object in its current configuration. It makes all properties readonly 
            and thens seals the object preventing any properties from changing their configuration. It is useful as 
            the ultimate lock-down for objects.
            @param obj Object to freeze
         */
        static native function freeze(obj: Object): Void

/* FUTURE 
        get options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
*/
        /** 
            Get an iterator for this object to be used by "for (v in obj)"
            @param options Iteration options. Reserved for future use
            @return An iterator object.
            @spec ejs
         */
        iterator native function get(options: Object? = null): Iterator

        /** 
            Get the prototype property descriptor. This returns the property descriptor for the named property in
            the given object. Property names are interpreted using the set of currently open namespaces. 
            @param obj Object to inspect
            @param prop Property name
            @return The a property descriptor object. Return null if the property is not found
         */
        static native function getOwnPropertyDescriptor(obj: Object, prop: String): Object

//  MOB -- inconsistent with JSON.baseClasses
        /** 
            Return an array of all property names including non-enumerable properties. This returns the bare names
            and does not include the namespace portions of the names. Use getOwnPropertyDescriptor to access property
            namespace values.
            @param obj Object to inspect
            @param options Property selection options
            @option includeBases Boolean determining if base class properties will included. Defaults to false.
            @option excludeFunctions Boolean determining if function properties will included. Defaults to false.
            @return Array of enumerable property names
         */ 
        static native function getOwnPropertyNames(obj: Object, options = null): Array

        /** 
            The number of properties in the object including non-enumerable properties.
         */
        static native function getOwnPropertyCount(obj: Object): Number

        /** 
            Get the prototype object
            @param obj Object to inspect
            @return The prototype object for the given object.
         */
        static native function getOwnPrototypeOf(obj: Object): Object

        /** 
            Get an iterator for this object to be used by "for each (v in obj)"
            @return An iterator object.
            @spec ejs
         */
        iterator native function getValues(): Iterator

        /** 
            Check if an object has a property. The property names is interpreted using the set of 
            currently open namespaces. 
            @param name Name of property to check for.
            @returns true if the object contains the specified property.
         */
        native function hasOwnProperty(name: String): Boolean

        /** 
            Test if an object is extensible
            @return True if the extensible trait of the object is true
         */
        static native function isExtensible(obj: Object): Boolean

        /** 
            Test if an object is frozen.
            @param obj Object to inspect
            @return True if the object is frozen.
         */
        static native function isFrozen(obj: Object): Boolean

        /** 
            Is this object a prototype of the nominated argument object.
            @param obj Target object to use in test.
            @returns true if this is a prototype of obj.
            TODO - should be on the prototype chain - Need a prototype ns
         */
        native function isPrototypeOf(obj: Object): Boolean

        /** 
            Test if an object is sealed.
            @param obj Object to inspect
            @return True if the object is sealed.
         */
        static native function isSealed(obj: Object): Boolean

        /** 
            Return an array of enumerable property names 
            @param obj Object to inspect
            @return Array of enumerable property names
         */ 
        static function keys(obj: Object): Array {
            let result = []
            for (key in obj) {
                result.append(key)
            }
            return result
        }

        /** 
            Prevent extensions to object. Sets the extensible property to false 
            @return The object argument to permit chaining.
         */
        static native function preventExtensions(obj: Object): Object

        /** 
            Test if the property is enumerable and visible in for/in traversals.  The property names is interpreted 
            using the set of currently open namespaces. 
            @param property Name of property to test.
            @returns true if this is a prototype of obj.
            TODO - prototype property
         */
        native function propertyIsEnumerable(property: String): Boolean

        /**
            The prototype object for objects of this type. The prototype object provides the template for instance 
            properties shared by all objects of a given type.
         */
        static native function get prototype(): Object
        static native function set prototype(v: Object): Void

        /** 
            Seal an object. This prevents changing the configuration of any property. This includes preventing property 
            deletion, changes to property descriptors or adding new properties to the object.
            @param obj Object to seal
         */ 
        static native function seal(obj: Object): Void

        /** 
            Convert an object to an equivalent JSON encoded string.
            @return This function returns an object literal string.
         */ 
        native function toJSON(): String

        /** 
            This function converts an object to a localized string representation. For many objects, this simply calls
            toString. If a class overrides, it may perform customized localization. For example: Date.toLocaleString
            will use the native O/S routines to encode localized dates.
            @returns a localized string representation of the object. 
         */ 
        native function toLocaleString(): String

        /** 
            This function converts an object to a string representation. Types typically override this to provide 
            the best string representation.
            @returns a string representation of the object. For Objects "[object className]" will be returned, 
            where className is set to the name of the class on which the object was based.
         */ 
        native function toString(): String

        /** 
          Return the value of the object
          @returns this object.
         */ 
        function valueOf(): String
            this


        /**
            Get the base type of a type object.
            @return A type object
         */
        native static function getBaseType(obj: Type): Type
  
        /**
            Get the name of the object if it is a function or type.
            @return The string name of the function or type
         */
        native static function getName(obj: Object): String

        /**
            Get the type for an object. If the object is an instance, this is the class type object. If the object is a
            type, this value is Type.
            @return A type object
         */
        native static function getType(obj: Object): Type
  
        /**
            Get the name of the type for an object. If the object is an instance, this is the name of the class type object.
            If the object is a type, this value is "Type". 
            @return A type object
         */
        native static function getTypeName(obj: Object): String
  
        /**
            Test if the object is a prototype object
            @return True if the object is being used as a prototype object
         */
        native static function isPrototype(obj: Object): Boolean
  
        /**
            Test if the object is a type object
            @return True if the object is a type object
         */
        native static function isType(obj: Object): Boolean
    }
  
    /**
        Return the name of a type. This is a fixed version of the standard "typeof" operator. It returns the real
        Ejscript underlying type name. This call is an alias for Object.getTypeName(o).
        @param o Object or value to examine. 
        @return A string type name. 
        @spec ejs
     */
    native function typeOf(o): String
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
