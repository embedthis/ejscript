/*
    Memory.es -- Memory statistics
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Singleton class to monitor and report on memory allocation and using.
        @stability evolving
        @spec ejs
     */
    class Memory {

        use default namespace public

        /**
            Total heap memory currently allocated by the application in bytes. This includes memory currently in use and 
            also memory that has been freed but is still retained by the application for future use. It does not include 
            code, static data or stack memory. If you require these, use the $resident call.
         */
        native static function get allocated(): Number

        //  TODO -- should use observers not callbacks
        /**
            Memory redline callback. When the memory redline limit is exceeded, the callback will be invoked. 
            If no callback is defined and the redline limit is exceeded, a MemoryError exception is thrown. This callback
            enables the application detect low memory conditions before they become critical and to recover by freeing 
            memory or to gracefully exit.  While the callback is active subsequent invocations of the callback are 
            suppressed.  The callback is invoked with the following signature:
                function callback(size: Number, total: Number): Void
         */
        native static function get callback(): Void
        native static function set callback(fn: Function): Void


        /**
            Maximum amount of heap memory the application may use in bytes. This defines the upper limit for heap memory 
            usage by the entire hosting application. If this limit is reached, subsequent memory allocations will fail and 
            a $MemoryError exception will be thrown. Setting it to zero will allow unlimited memory allocations up 
            to the system imposed maximum. If $redline is defined and non-zero, the redline callback will be invoked 
            when the $redline is exceeded. By default, maximum is set to unlimited.
         */
        native static function get maximum(): Number

        /**
            @duplicate Memory.maximum
            @param value New maximum value in bytes
         */
        native static function set maximum(value: Number): Void

        /**
            Memory redline value in bytes. When the memory redline limit is exceeded, the redline $callback will be invoked. 
            If no callback is defined, a MemoryError exception is thrown. The redline limit enables the application detect 
            low memory conditions before they become critical and to recover by freeing memory or to gracefully exit. 
            Note: the redline applies to the entire hosting application.
         */
        native static function get redline(): Number

        /**
            @duplicate Memory.redline
            @param value New memory redline limit in bytes
         */
        native static function set redline(value: Number): Void

        //  MOB BUG
        /**
            Application's current resident set in bytes. This is the total memory used to host the application and 
            includes all the the application code, data and heap. It is measured by the O/S.
            NOTE: this is currently reporting the peak resident memory and not the current resident memory.
         */
        native static function get resident(): Number

        /**
            System RAM. This is the total amount of RAM installed in the system in bytes.
         */
        native static function get system(): Number
        
        //  TODO - should go to stream
        /**
            Prints memory statistics to stdout. This is primarily used during development for performance measurement.
         */
        native static function stats(): void
    }
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
