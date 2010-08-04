/*
    Template.es -- Ejscript templated web content handler
 */

module ejs.web {
    /** 
        Template middleware filter. This interprets the output of an inner web app as a template page which is processed.
        @param app Application function object
        @returns A response object hash
     */
    function TemplateFilter(app: Function): Object {
        return function(request) {
            let response = app(request)
            let id = md5(request.id)
            return Loader.load(id, id, request.config, function (id, path) {
                if (!global.TemplateParser) {
                    load("ejs.web.template.mod")
                }
                let data = TemplateParser().build(response.body)
                return Loader.wrap(data)
            }).app(request)
        }
    }

    /**
        Build a web application from a template page. The template web page at Request.filename will be processed and
            a web application script created.
        @param request Request object
        @return A web application function
     */
    function TemplateApp(request: Request): Function {
        let app = TemplateBuilder(request)
        return app(request)
    }

    /** 
        Template builder for use in routing tables to serve requests for template files (*.ejs).
        @param request Request object. 
        @return A web script function that services a web request.
        @example:
          { name: "index", builder: TemplateBuilder, match: "\.ejs$" }
     */
    function TemplateBuilder(request: Request): Function {
        let path = request.filename
        if (!path.exists) {
            request.writeError(Http.NotFound, "Cannot find " + path)
            return null
        }
        return Loader.load(path, path, request.config, function (id, path) {
            if (!global.TemplateParser) {
                load("ejs.web.template.mod")
            }
            let data = TemplateParser().build(path.readString())
            return Loader.wrap(data)
        }).app
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
