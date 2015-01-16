/*
    Script.es -- Ejscript templated web content handler
 */

# Config.WEB
module ejs.web {

    /**
        Run a script app. Run the script specified by request.filename
        @param request Request object
        @return A response object
        @example:
          { name: "index", builder: ScriptBuilder, match: "\.es$" }
        @spec ejs
        @stability prototype
     */
    function ScriptApp(request: Request) {
        let app = ScriptBuilder(request)
        return app.call(request, request)
    }


    //  TODO -- make all builder functions lower case: scriptBuilder -- or inline above
    /** 
        Script builder to create a function to serve a script request (*.es).  
        @param request Request object. 
        @return A request response
        @spec ejs
        @stability prototype
     */
    function ScriptBuilder(request: Request): Object {
        if (!request.filename.exists) {
            request.writeError(Http.NotFound, "Cannot find " + escapeHtml(request.pathInfo)) 
            //  TODO - should not need throw, just return
            throw true
        }
        try {
            return Loader.require(request.filename, request.config).app
        } catch (e) {
            request.writeError(Http.ServerError, e)
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
