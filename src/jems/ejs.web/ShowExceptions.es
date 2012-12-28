/*
    Show exceptions to the client 
 */

module ejs.web {
    /** 
        ShowExceptions middleware wrapper. This catches exceptions and formats the result back to the client.
        @param app Application servicing the request and generating the response.
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
     */
    function ShowExceptions(app: Function): Function {
        return function(request: Request) {
            try {
                return app(request)
            } catch (e) {
                return {
                    status: Http.ServerError,
                    body: errorBody(typeOf(e), e.message + "\r\n" + e.formatStack()),
                }
            }
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
