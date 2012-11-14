/*
    XMLList.es - XMLList class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        The XMLList class is a helper class for the XML class.
        @spec ejs
        @hide
     */
    final class XMLList {

        use default namespace public

        /**
            XML Constructor. Create an empty XML object.
         */
        native function XMLList() 

        # FUTURE
        native function addNamespace(ns: Namespace): XML

        /**
            Append a child to this XML object.
            @param child The child to add.
            @return This object with the added child.
         */
        function appendChild(child: XML): XML {
            this[child.name()] = child
            return this
        }

        /**
            Get an XMLList containing all of the attributes of this object with the given name.
            @param name The name to search on.
            @return An XMLList with all the attributes (zero or more).
         */
        function attribute(name: String): XMLList
            this["@" + name]

        /**
            Get an XMLList containing all of the attributes of this object.
            @return An XMLList with all the attributes (zero or more).
         */
        function attributes(): XMLList
            this.@*

        /**
            Get an XMLList containing the list of children (properties) in this XML object with the given name.
            @param name The name to search on.
            @return An XMLList with all the children names (zero or more).
         */
        function child(name: String): XMLList {
            if (name.isDigit) {
                return this[name cast Number]
            } else {
                return this[name]
            }
        }

        /**
            Get the position of this object within the context of its parent.
            @return A number with the zero-based position.
         */
        function childIndex(): Number {
            let p = parent()
            for (i in p) {
                if (p[i] === this) {
                    return i
                }
            }
            return -1
        }

        /**
            Get an XMLList containing the children (properties) of this object in order.
            @return An XMLList with all the properties.
         */
        function children(): XMLList
            this.*
        
        /**
            Get an XMLList containing the properties of this object that are
            comments.
            @return An XMLList with all the comment properties.
         */
        # FUTURE
        native function comments(): XMLList
        
        /**
            Compare an XML object to another one or an XMLList with only one XML object in it. If the 
            comparison operator is true, then one object is said to contain the other.
            @return True if this object contains the argument.
         */
        # FUTURE
        native function contains(obj: Object): Boolean

        /**
            Deep copy an XML object. The new object has its parent set to null.
            @return Then new XML object.
         */
        function copy(): XML
            this.clone(true)

        /**
            Get the defaults settings for an XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting. 
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function defaultSettings(): Object

        /**
            Get all the descendants (that have the same name) of this XML object. The optional argument defaults 
            to getting all descendants.
            @param name The (optional) name to search on.
            @return The list of descendants.
         */
        function descendants(name: String = "*"): Object
            this["." + name]

        /**
            Get all the children of this XML node that are elements having the
            given name. The optional argument defaults to getting all elements.
            @param name The (optional) name to search on.
            @return The list of elements.
         */
        function elements(name: String = "*"): XMLList
            this[name]

        /**
            Get an iterator for this node to be used by "for (v in node)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this node to be used by "for each (v in node)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Determine whether this XML object has complex content. If the object has child elements it is 
            considered complex.
            @return True if this object has complex content.
         */
        function hasComplexContent(): Boolean
            this.*.length() > 0

        /**
            Determine whether this object has its own property of the given name.
            @param name The property to look for.
            @return True if this object does have that property.
         */
        override function hasOwnProperty(name: String): Boolean
            this[name] != null

        /**
            Determine whether this XML object has simple content. If the object
            is a text node, an attribute node or an XML element that has no
            children it is considered simple.
            @return True if this object has simple content.
         */
        # FUTURE
        native function hasSimpleContent(): Boolean

        /**
            Return the namespace in scope
            @return Array of namespaces
         */
        function inScopeNamespaces(): Array
            this.*.length() == 0

        /**
            Insert a child object into an XML object immediately after a specified marker object. If the marker 
            object is null then the new object is inserted at the beginning. If the marker object is not found 
            then the insertion is not made.
            @param marker Insert the new element after this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildAfter(marker: Object?, child: Object): XML

        /**
            Insert a child object into an XML object immediately before a specified marker object. If the marker 
            object is null then the new object is inserted at the end. If the marker object is not found then the
            insertion is not made.
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildBefore(marker: Object?, child: Object): XML

        /**
            Return the length of an XML object in elements. NOTE: this is a method and must be invoked with ().
            @return A number indicating the number of child elements.
         */
        native function length(): Number

        /**
            Get the local name portion of the complete name of this XML object.
            @return The local name.
         */
        # FUTURE
        native function localName(): String

        /**
            Get the qualified name of this XML object.
            @return The qualified name.
         */
        native function name(): String

        # FUTURE
        native function namespace(prefix: String? = null): Object

        # FUTURE
        native function namespaceDeclarations(): Array

        /**
            Get the kind of node this XML object is.
            @return The node kind.
         */
        # FUTURE
        native function nodeKind(): String

        /**
            Merge adjacent text nodes into one text node and eliminate empty text nodes.
            @return This XML object.
         */
        # FUTURE
        native function normalize(): XML

        /**
            Get the parent of this XML object.
            @return The parent.
         */
        native function parent(): XML

        /**
            Insert a child object into an XML object immediately before all existing properties.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function prependChild(child: Object): XML

        /**
            Get all the children of this XML node that are processing instructions having the given name. 
            The optional argument defaults to getting all processing instruction nodes.
            @param name The (optional) name to search on.
            @return The list of processing instruction nodes.
         */
        # FUTURE
        native function processingInstructions(name: String = "*"): XMLList

        /**
            Test a property to see if it will be included in an enumeration over the XML object.
            @param property The property to test.
            @return True if the property will be enumerated.
         */
        # FUTURE
        override native function propertyIsEnumerable(property: Object): Boolean

        /**
            Change the value of a property. If the property is not found, nothing happens.
            @param property The property to change.
            @param value The new value.
            @return True if the property will be enumerated.
         */
        # FUTURE
        native function replace(property: Object, value: Object): void
        
        /**
            Replace all the current properties of this XML object with a new set. The argument can be 
            another XML object or an XMLList.
            @param properties The new property or properties.
            @return This XML object.
         */
        # FUTURE
        native function setChildren(properties: Object): XML

        /**
            Set the local name portion of the complete name of this XML object.
            @param The local name.
         */
        # FUTURE
        native function setLocalName(name: String): void

        /**
            Set the qualified name of this XML object.
            @param The qualified name.
         */
        # FUTURE
        native function setName(name: String): void

        /**
            Get the settings for this XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function settings(): Object

        /**
            Configure the settings for this XML object.
            @param settings A "settings" object previously returned from a call to the "settings" method.
         */
        # FUTURE
        native function setSettings(settings: Object): void
        
        /**
            Get all the properties of this XML node that are text nodes having the given name. The optional 
            argument defaults to getting all text nodes.
            @param name The (optional) name to search on.
            @return The list of text nodes.
         */
        # FUTURE
        native function text(name: String = "*"): XMLList
        
        /**
            Convert to a JSON encoding
            @return A JSON string 
         */
        override native function toJSON(): String 

        /**
            Provides a string representation of the XML object.
            @return A string with the encoding.
         */
        override native function toString(): String 

        /**
            Provides an XML-encoded version of the XML object that includes the tags.
            @return A string with the encoding.
         */
        # FUTURE
        native function toXMLString(): String 

        /**
            Return this XML object.
            @return This object.
         */
        override function valueOf(): XML
            this

        /*
           XML
                .prototype
                .ignoreComments
                .ignoreProcessingInstructions
                .ignoreWhitespace
                .prettyPrinting
                .prettyIndent
                .settings()
                .setSettings(settings)
                .defaultSettings()

               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
           XMLList
               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
         */
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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
