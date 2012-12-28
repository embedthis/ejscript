/**
    Iterator.es -- Iteration support via the Iterable interface and Iterator class. 

    This provides a high performance native iterator for native classes. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Iterable is an internal interface used by native types to provide iterators for use in for/in statements.
        @hide
        @stability evolving
     */
    iterator interface Iterable {
        use default namespace iterator

        /**
            Get an Iterator for use with "for ... in"
            @return An Iterator
         */
        function get(): Iterator

        /**
            Get an Iterator for use with "for each ... in"
            @return An Iterator
         */
        function getValues(): Iterator
    }

    /**
        Iterator is a helper class to implement iterators.
        @hide
     */
    iterator final class Iterator {

        use default namespace public

        /**
            Return the next element in the object.
            @return An object
            @throws StopIteration
         */
        native function next(): Object
    }

    /** 
        StopIteration class. Iterators throw the StopIteration class instance to signal the end of iteration in for/in loops.
        @spec ejs
     */
    iterator final class StopIteration {}
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
