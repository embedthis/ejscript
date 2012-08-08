/**
    Array.es - Array class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Arrays provide a growable, integer indexed, in-memory store for objects.  An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered).  Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list.
        @stability evolving
     */
    dynamic class Array {

        use default namespace public

        /**
            Create a new array.
            @param values Initialization values. The constructor allows three forms:
            <ul>
                <li>Array()</li>
                <li>Array(size: Number)</li>
                <li>Array(elt: Object, ...args)</li>
            </ul>
         */
        native function Array(...values)

        /**
            Append an item to the array.
            @param obj Object to add to the array 
            @return The array itself.
            @spec ejs
         */
        native function append(obj: Object): Array

        /**
            Clear an array. Remove all elements of the array.
            @spec ejs
         */
        native function clear() : Void

        /**
            Clone the array and all its elements.
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            @return A new array
            @spec ejs
         */
        override native function clone(deep: Boolean = true) : Array

        /**
            Compact an array. Remove all null elements.
            @return Modified original array
            @spec ejs
         */
        native function compact() : Array

        /**
            Concatenate the supplied elements with the array to create a new array. If any arguments specify an 
            array, their elements are concatenated. This is a one level deep copy.
            @param args A variable length set of values of any data type.
            @return A new array of the combined elements.
         */
        native function concat(...args): Array 

        /**
            See if the array contains an element using strict equality "===". This call searches from the start of the 
            array for the specified element. Note that some types are not interned like strings and so contains may
            return false if a different object with the same value is contained in the array. This can happen with Path
            types.
            @param element The object to search for.
            @return True if the element is found. Otherwise false.
            @spec ejs
         */
        function contains(element: Object): Boolean
            indexOf(element) >= 0

        /**
            Determine if all elements match.
            Iterate over every element in the array and determine if the matching function is true for all elements. 
            This method is lazy and will cease iterating when an unsuccessful match is found. The checker is called 
            with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function always returns true.
         */
        function every(match: Function): Boolean {
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    return false
                }
            }
            return true
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The match is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            This method is identical to the @findAll method.
            @param match Matching function
            @return Returns a new array containing all matching elements.
         */
        function filter(match: Function): Array
            findAll(match)

        /**
            Find the first matching element.
            Iterate over elements in the object and select the first element for which the matching function returns true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return The matched item
            @spec ejs
         */
        function find(match: Function): Object? {
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return this[i]
                }
            }
            return null
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return Returns an array containing all matching elements.
            @spec ejs
         */
        function findAll(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Transform all elements.
            Iterate over the elements in the array and transform all elements by applying the transform function. 
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param callback Transforming function
            @param thisObj Object to use for the "this" value when invoking the callback
            @return Returns the original (transformed) array.
         */
        function forEach(callback: Function, thisObj: Object? = null): Array {
            if (thisObj) {
                for (let i: Number in this) {
                    callback.call(thisObj, this[i], i, this)
                }
            } else {
                for (let i: Number in this) {
                    callback(this[i], i, this)
                }
            }
            return this
        }

        /**
            Iterator for this array to be used by "for (v in array)"
         */
        override iterator native function get(): Iterator

        /**
            Iterator for this array to be used by "for each (v in array)"
         */
        override iterator native function getValues(): Iterator

        /**
            Search for an item using strict equality "===". This call searches from the start of the array for 
            the specified element.  
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to zero). If the index 
                is negative, it is taken as an offset from the end of the array. If the calculated index is less than 
                zero the entire array is searched. If the index is greater than the length of the array, -1 is returned.
            @return The items index into the array if found, otherwise -1.
         */
        native function indexOf(element: Object, startIndex: Number = 0): Number

        /**
            Insert elements. Insert elements at the specified position. The insertion occurs before the element at the 
                specified position. Negative indicies will measure from the end so that -1 will specify the last element.  
                Indicies greater than the array length will append to the end. Indicies before the first position will
                insert at the start.
            @param pos Where in the array to insert the objects.
            @param ...args Arguments are aggregated and passed to the method in an array.
            @return The original array.
            @spec ejs
         */
        native function insert(pos: Number, ...args): Array

        /**
            Convert the array into a string.
            Joins the elements in the array into a single string by converting each element to a string and then 
            concatenating the strings inserting a separator between each.
            @param sep Element separator.
            @return A string.
         */
        native function join(sep: String = ""): String

        /**
            Find an item searching from the end of the array.
            Search for an item using strict equality "===". This call searches from the end of the array for the given 
            element.
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to the array's length).
                If the index is negative, it is taken as an offset from the end of the array. If the calculated index 
                is less than zero, -1 is returned. If the index is greater or equal to the length of the array, the
                whole array will be searched.
            @return The items index into the array if found, otherwise -1.
         */
        native function lastIndexOf(element: Object, startIndex: Number = -1): Number

        /**
            Length of an array.
         */
        native function get length(): Number

        /**
            Set the length of an array. The array will be truncated if required. If the new length is greater then 
            the old length, new elements will be created as required and set to undefined. If the new length is less
            than 0 the length is set to 0.
            @param value The new length
         */
        native function set length(value: Number): Void

        /**
            Call the mapper on each array element in increasing index order and return a new array with the values returned 
            from the mapper. The mapper function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
         */
        function map(mapper: Function): Array {
            var result: Array = clone()
            result.transform(mapper)
            return result
        }

        /**
            Remove and return the last value in the array.
            @return The last element in the array. Returns undefined if the array is empty
         */
        native function pop(): Object?

        /**
            Append items to the end of the array.
            @param items Items to add to the array.
            @return The new length of the array.
         */
        native function push(...items): Number 

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            left to right. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduce(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = 0
                if (initial) {
                    previous = callback(initial, this[i++], 0, this)
                }
                for (; i < length; i++) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            right to left. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduceRight(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = length - 1
                if (initial) {
                    previous = callback(initial, this[i--], 0, this)
                }
                for (; i >= 0; i--) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Find non-matching elements. Iterate over all elements in the array and select all elements for which 
            the filter function returns false. The matching function is called with the following signature:
          
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
          
            @param match Matching function
            @return A new array of non-matching elements.
            @spec ejs
         */
        function reject(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Remove elements. Remove the elements from $start to $end inclusive. The elements are removed and not just set 
            to undefined as the delete operator will do. Indicies are renumbered. NOTE: this routine delegates to splice.
            @param start Numeric index of the first element to remove. Negative indices measure from the end of the array.
            -1 is the last element.
            @param end Numeric index of the last element to remove
            @spec ejs
         */
        function remove(start: Number, end: Number = -1): Void {
            if (start < 0) {
                start += length
            }
            if (end < 0) {
                end += length
            }
            splice(start, end - start + 1)
        }

        /**
            Remove specified elements from the array. The elements are removed and not just set 
            to undefined as the delete operator will do. Indicies are renumbered. 
            @param elts List of elements to remove.
            @spec ejs
         */
        native function removeElements(...elts): Void

        /**
            Reverse the order of the objects in the array. The elements are reversed in the original array.
            @return A reference to the array.
         */
        native function reverse(): Array 

        /**
            Remove and return the first element in the array.
            @returns the previous first element in the array.
         */
        native function shift(): Object 

        /**
            Create a new array subset by taking a slice from an array.
            @param start The array index at which to begin. Negative indicies will measure from the end so that -1 will 
                specify the last element. If start is greater than or equal to end, the call returns an empty array.
            @param end The array index at which to end. This is one beyond the index of the last element to extract. If 
                end is negative, it is measured from the end of the array, so use -1 to mean up to but not including the 
                last element of the array.
            @param step Slice every step (nth) element. The step value may be negative.
            @return A new array that is a subset of the original array.
         */
        native function slice(start: Number, end: Number = -1, step: Number = 1): Array 

        /**
            Determine if some elements match.
            Iterate over all element in the array and determine if the matching function is true for at least one element. 
            This method is lazy and will cease iterating when a successful match is found.
            The match function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function ever is true.
         */
        function some(match: Function): Boolean {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return true
                }
            }
            return false
        }

        /**
            Sort the array. The array is sorted in lexical order. A compare function may be supplied.
            @param compare Function to use to compare. A null comparator will use a text compare. The compare signature is:
                function comparator (array: Array, index1: Number, index2: Number): Number
                The comparison function should return 0 if the items are equal, -1 if the item at index1 is less and should
                return 1 otherwise.
            @param order If order is >= 0, then an ascending lexical order is used. Otherwise descending.
            @return the sorted array reference
            @spec ejs Added the order argument.
         */
        native function sort(compare: Function? = null, order: Number = 1): Array 

        /**
            Insert, remove or replace array elements. Splice modifies an array in place. 
            @param start The array index at which the insertion or deleteion will begin. Negative indicies will measure 
                from the end so that -1 will specify the last element.  
            @param deleteCount Number of elements to delete. If omitted, splice will delete all elements from the 
                start argument to the end.  
            @param values The array elements to add.
            @return Array containing the removed elements.
         */
        native function splice(start: Number, deleteCount: Number, ...values): Array 

        /**
            Convert an array to an equivalent JSON encoding.
            @return This function returns an array literal string.
            @throws TypeError If the array could not be converted to a string.
            NOTE: currently using Object.toJSON for this capability
         */ 
        #FUTURE
        override native function toJSON(): String

        # FUTURE
        override native function toLocaleString(): String 

        /**
            Convert the array to a single string each member of the array has toString called on it and the resulting 
            strings are concatenated.
            @return A string
         */
        override native function toString(): String 

        /**
            Transform all elements.
            Iterate over all elements in the object and transform the elements by applying the transform function. 
            This modifies the object elements in-situ. The transform function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
            @spec ejs
         */
        function transform(mapper: Function): Array {
            for (let i: Number in this) {
                this[i] = mapper(this[i], i, this);
            }
            return this
        }

        /**
            Remove duplicate elements and make the array unique. Duplicates are detected by using "==" (ie. content 
                equality, not strict equality).
            @return The original array with unique elements
            @spec ejs
         */
        native function unique(): Array

        /**
            Insert the items at the front of the array.
            @param items to insert
            @return Returns the array reference
         */
        native function unshift(...items): Array

        /**
            Array intersection. Return the elements that appear in both arrays. 
            @param arr The array to join.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function & (arr: Array): Array

        /**
            Append. Appends elements to an array.
            @param elements The array to add append.
            @return The modified original array.
            @spec ejs
         */
        # DOC_ONLY
        native function << (elements: Array): Array

        //  MOB - need a function equivalent for this
        /**
            Array subtraction. Remove any items that appear in the supplied array.
            @param arr The array to remove.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function - (arr: Array): Array

        /**
            Array union. Return the union of two arrays. 
            @param arr The array to join.
            @return A new array
            @spec ejs
         */
        # DOC_ONLY
        native function | (arr: Array): Array

        /**
            Concatenate two arrays. 
            @param arr The array to add.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function + (arr: Array): Array
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
