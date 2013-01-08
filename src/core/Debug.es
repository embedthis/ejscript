/*
    Debug.es -- Debug class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Debug configuration class. Singleton class containing the application's debug configuration.
        @spec ejs
        @stability prototype
     */
    class Debug {

        use default namespace public

        /**
            Break to the debugger. Suspend execution and break to the debugger.
         */ 
        native static function breakpoint(): void

        /**
            The current debug mode.  Setting mode to true will put the application in debug mode. When debug mode 
            is enabled, the runtime will typically suspend timeouts and will take other actions to make debugging easier.
            Invoking the ejs shell with a -D command line switch will also enable debug mode.
            This property is read-write. 
         */
        native static function get mode(): Boolean

        /**
            @duplicate Debug.mode
            @param value True to turn debug mode on or off.
         */
        native static function set mode(value: Boolean): void
    }

    /** 
        Convenient way to trap to the debugger
     */
    native function breakpoint(): Void
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
