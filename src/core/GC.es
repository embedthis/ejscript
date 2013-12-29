/*
    GC.es -- Garbage collector class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Garbage collector control class. Singleton class to control operation of the Ejscript garbage collector.
        @spec ejs
        @stability evolving
     */
    class GC {

        use default namespace public

        /**
            Is the garbage collector is enabled.  Enabled by default.
         */
        native static function get enabled(): Boolean
        native static function set enabled(on: Boolean): Void

        /**
            The quota of work to perform before the GC will be invoked. Set to the number of work units that will 
            trigger the GC to run. This roughly corresponds to the number of allocated objects.
         */
        native static function get newQuota(): Number
        native static function set newQuota(quota: Number): Void

        /**
            Run the garbage collector and reclaim memory allocated to objects and properties that are no longer reachable. 
            When objects and properties are freed, any registered native destructors will be called. The run function will 
            run the garbage collector even if the $enable property is set to false. 
            It is normally not required to manually call the $run method as the ejs virtual machine runtime will 
            automatically run the garbage collector as required.
            @param deep If set to true, will collect from all generations. The default is to collect only the youngest
                geneartion of objects.
         */
        native static function run(deep: Boolean = false): void

        /**
            Verify memory. In debug builds, this call verifies all memory blocks by checking a per-block signature.
            This is very slow, so call sparingly. In release builds, this call does nothing.
            UNUSED - remove
            @hide
         */
        native static function verify(): Void

    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
