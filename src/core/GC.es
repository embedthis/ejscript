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
         */
        native static function verify(): Void

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
    details at: http://embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://embedthis.com 
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
