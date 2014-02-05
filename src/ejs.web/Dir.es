/*
    Dir.es - Directory content handler
 */

# Config.WEB
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
                    let route = request.route.router.route(request)
                    return request.server.process(route.response, request)
                }
            }
            return { 
                status: Http.NotFound, 
                body: errorBody("Not Found", "Cannot locate " + escapeHtml(request.pathInfo))
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
        UNUSED
    function DirBuilder(request: Request): Function {
        return DirApp
    }
     */
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
