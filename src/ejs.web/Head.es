/**
    Head.es - Respond to HEAD requesets and Return just the headers and omit the body.
    NOTE: This is typically done by good web servers anyway.
    Usage:
        require ejs.web
        exports.app = Head(app)
 */

# Config.WEB
module ejs.web {
    /** 
        Head wrapper middleware. Return the headers and omit the body for HTTP HEAD requests. HEAD requests should still
        preserve the original Content-Length header value even though they transmit no body content.
        This version is limited, in that it will only define a Content-Length if the response body is a string. 
        @param app Application servicing the request and generating the response.
        @return A web application function that when invoked with the request will yield a response object.
        @example:
            export.app = Head(app)
     */
    function Head(app: Function) {
        return function(request) {
            var response = app(request)
            if (request.method == "HEAD") {
                if (response.body is String) {
                    let length = response.body.length
                    response.headers ||= {}
                    blend(response.headers, {"Content-Length": length})
                }
                response.body = null
            }
            return response
        }
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
