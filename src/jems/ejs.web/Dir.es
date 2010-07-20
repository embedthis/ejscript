/*
    Dir.es - Directory content handler
 */

module ejs.web {

    /** 
        Directory content handler. This redirects requests for directories and appends configured index files.
        @spec ejs
        @stability prototype
     */
    class Dir {
        use default namespace public

        /** 
            Load a directory request. If the request ends with "/", the request is transparently redirected to an index.
            @param request Request object
            @returns An exports object with an "app" property representing the application, or returns null if the
                request has been redirected and must be re-routed.
         */
        static function load(request: Request): Object {
            let results
            if (request.pathInfo.endsWith("/")) {
                let path = request.dir.join(request.pathInfo.trimStart('/'))
                for each (index in request.server.indicies) {
                    let p = path.join(index)
                    if (p.exists) {
                        /* Return a String containing the new pathInfo to serve */
                        request.pathInfo += index 
                        requst.route.router.route(request)
                        return null
                    }
                }
                results = { 
                    status: Http.NotFound, 
                    body: errorBody("Not Found", "Can't locate " + escapeHtml(request.pathInfo))
                }
            } else {
                results = { 
                    status: Http.MovedPermanently,
                    headers: { location: request.uri + "/" }
                    body: errorBody("Moved Permanently", 
                        'The document has moved <a href="' + request.pathInfo + "/" + '">here</a>.')
                }
            }
            return { 
                app: function (request) {
                    return results
                }
            }
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
