/*
    Dir.es - Directory content handler
 */

module ejs.web {

    /** 
        Directory content handler. This redirects requests for directories and serves directory index files.
        If the request pathInfo ends with "/", the request is transparently redirected to an index file if one is present.
        The set of index files is defined by HttpServer.indicies. If the request is a directory but does not end in "/",
        the client is redirected to a URL equal to the pathInfo with a "/" appended.
        @param request Request object
        @returns A response hash object
        @spec ejs
        @stability prototype
     */
    function DirApp(request: Request): Object {
        if (request.pathInfo.endsWith("/")) {
            for each (index in request.server.indicies) {
                let path = request.filename.join(index)
                if (path.exists) {
                    /* Return a String containing the new pathInfo to serve */
                    request.pathInfo += index 
                    app = request.route.router.route(request)
                    return Web.process(app, request)
                }
            }
            return { 
                status: Http.NotFound, 
                body: errorBody("Not Found", "Can't locate " + escapeHtml(request.pathInfo))
            }
        }
        return { 
            status: Http.MovedPermanently,
            headers: { location: request.uri + "/" }
            body: errorBody("Moved Permanently", 'The document has moved <a href="' + request.pathInfo + "/" + '">here</a>.')
        }
    }

    /** 
        Directory builder for use in routing tables to service requests for directories. 
        @param request Request object. 
        @return A web application function that services a web request and when invoked with the request object will 
        yield a response object.
        @example:
          { name: "index", builder: DirBuilder, match: Router.isDir }
        @spec ejs
        @stability prototype
     */
    function DirBuilder(request: Request): Function {
        return DirApp
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
    
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=8 ts=8 expandtab

    @end
 */
