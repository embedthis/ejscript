/*
    MethodOverride.es - Override the method type based on the Method-Override header
 */

# Config.WEB
module ejs.web {

    /**
        Method override wrapper middleware. Provides HTTP method overriding via a "-ejs-method-" POST parameter or via
            a X-HTTP-METHOD-OVERRIDE Http header.
        @param app Application generating the response. 
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = MethodOverride(app)
     */
    function MethodOverride(app: Function): Function {
        return function(request: Request) {
            if (request.method == "POST") {
                let method = request.params["-ejs-method-"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method) {
                    request.method = method
                }
            }
            return app(request)
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
