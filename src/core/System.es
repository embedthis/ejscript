/*
    System.es - System class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        FUTURE
            endian
     */
    /**
        System is a utility class providing methods to interact with the operating system.
        @spec ejs
        @stability prototype
     */
    enumerable class System {

        use default namespace public

        /**
            Default buffer size 
         */
        public static const Bufsize: Number = 1024

        /**
            The fully qualified system hostname
         */
        native static function get hostname(): String

        /**
            The system IP Address
            @hide
         */
        native static function get ipaddr(): String

        /** @hide */
        native static function get tmpdir(): String
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
