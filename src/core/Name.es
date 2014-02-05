/*
    Name.es -- Name class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        @hide
        @stability prototype
     */
    # FUTURE
    final class Name {
        use default namespace public

        const qualifier: Namespace
        const identifier: Namespace

        native function Name(qual: String, id: String? = undefined)
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
