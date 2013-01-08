/*
    Locale.es - Locale specific defaults and control

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*  TODO
    Calendar Date Time
    Currency
*/

module ejs {

    /**
        Locale information.
        @stability prototype
        @hide
     */
    class Locale {

        use default namespace public

        static const textEncoding = "utf-8"

        /*  
            Configure the locale
            @param language See http://www.ics.uci.edu/pub/ietf/http/related/iso639.txt
            @param country  See http://www.chemie.fu-berlin.de/diverse/doc/ISO_3166.html
            @hide
         */
        function Locale(language: String, country: String, variant: String) {}

        # FUTURE
        var config = {
            formats: {
                currency:   "$%10f",
                Date:       "%a %e %b %H:%M",
            },
        }
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
