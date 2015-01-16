/*
    Frame.es -- Frame class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Function call frame class. The Frame type is used internally by the ejs virtual machine to manage
        function calls and store function local variables.
        @stability evolving
     */
    dynamic class Frame extends Function { }
}


/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
