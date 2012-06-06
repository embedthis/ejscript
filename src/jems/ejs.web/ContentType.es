/*
    ContentType.es -- Define Content-Type headers
 */

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
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
