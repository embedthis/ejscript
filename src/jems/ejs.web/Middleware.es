/*
    Middleware.es - Wrap a web app with defined middleware
 */

module ejs.web {

    /** 
        Define middleware for a web app. This wrapps the web application function with defined middleware filters.
        @param app Base web app function object
        @param middleware Array of middleware wrapper applications
        @returns A top level web application function object
        @spec ejs
        @stability prototype
     */
    function Middleware(app: Function, middleware: Array? = null): Function {
        for each (mid in middleware) {
            app = mid[i](app)
        }
        return function (request: Request)
            app(request)
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
