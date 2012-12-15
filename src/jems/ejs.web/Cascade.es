/*
    Cascade slices and return the response from the first non-404 slice
    Usage:
        require ejs.web
        exports.app = Cascade(app)
 */

module ejs.web {
    /** 
        Cascade middleware script for web apps.
        @param apps Array of applications to try in turn. The first app to return a valid status (!= Http.NotFound) 
        will conclude the request.
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = Cascade(app1, app2, app3)
     */
    function Cascade(...apps): Object? {
        return function(request) {
            for each (app in apps) {
                let response = app(request)
                if (response.status != Http.NotFound) {
                    return response
                }
            }
            return null
        }
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
