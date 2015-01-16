/*
    Null.es -- Null class used for the null value.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Base type for the null value. There is only one instance of the Null type and that is the null value.
        @spec ejs
        @stability evolving
     */
    final class Null {

        /* WARNING: Do not add properties here. Null must have no properties beyond those inherited by Object */

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function get(): Iterator

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function getValues(): Iterator
    }
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
