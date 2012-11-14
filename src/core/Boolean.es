/*
    Boolean.es -- Boolean class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Boolean class from which to create true and false values.
        @stability stable
     */
    final class Boolean {
        /**
            Boolean constructor. Construct a Boolean object and initialize its value. Since Boolean values are 
            immutable, this constructor will return a reference to either the "true" or "false" values.
            @param value Optional value to use in creating the Boolean object. If the value is omitted or 0, -1, NaN,
                false, null, undefined or the empty string, then the object will be created and set to false.
         */
        native function Boolean(value: Boolean? = false)
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.

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
