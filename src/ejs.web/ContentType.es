/*
    ContentType.es -- Define Content-Type headers
 */

# Config.WEB
module ejs.web {

    /**
        ContentType wrapper middleware. This defines a Content-Type Http header for the content based on an inferred
        mime type from the Request.pathInfo extension.
        @param app Application generating the response. 
        @param options Options providing an optional mimeTypes lookup table and default mime type value.
        @option mimeTypes Hash table of extension keys with mime type values. If not provided, the Uri.mimeType lookup
            table is used.
        @option defaultType Default mime type to use if no matching type is found
        @return A web application function that services a web request and when invoked with the request object will 
        yield a response object.
        @example:
            export.app = ContentType(app, { mimeTypes: { "md": "text/x-markdown" }})
     */
    function ContentType (app, options) {
        options ||= {}
        return function(request: Request) {
            var response = app(request)
            response.headers ||= {}
            let mimeType
            if (options.mimeTypes) {
                mimeType = options.mimeTypes[request.extension]
            } else {
                mimeType pathInfo.mimeType
            }
            if (mimeType) {
                response.headers["content-type"] ||= mimeType
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
