/*
    MethodOverride.es - Override the method type based on the Method-Override header
 */

module ejs.web {

    /**
        Method override wrapper middleware. Provides HTTP method overriding via a "__method__" POST parameter or via
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
                let method = request.params["__method__"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method) {
                    //MOB request.originalMethod ||= request.method
                    request.method = method
                }
            }
            return app(request)
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
