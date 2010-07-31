/**
    Head.es - Respond to HEAD requesets and Return just the headers and omit the body.
    NOTE: This is typically done by good web servers anyway.
    Usage:
        require ejs.web
        exports.app = Head(app)
 */

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
                    blend(response.headers, {"Content-Length": length}, true)
                }
                response.body = null
            }
            return response
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
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

    @end
 */
