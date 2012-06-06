/*
    Template.es -- Ejscript templated web content handler
 */

module ejs.web {
    /** 
        Template middleware filter. This interprets the output of an inner web app as a template page which is processed.
        @param app Application function object
        @returns A response object hash
        @spec ejs
        @stability prototype
     */
    function TemplateFilter(app: Function): Object {
        return function(request) {
            let response = app(request)
            let id = md5(request.id)
            return Loader.load(id, id, request.config, function (id, path) {
                if (!global.TemplateParser) {
                    load("ejs.template.mod", {reload: false})
                }
                let data = TemplateParser().build(response.body)
                return Loader.wrap(id, data)
            }).app(request)
        }
    }

    /** 
        Template web page handler. The template web page at request.filename will be processed and run.
        @param request Request objects 
        @returns A response hash object
        @spec ejs
        @example: Example use in a Route table entry
          { name: "index", builder: TemplateApp, match: "\.ejs$" }
        @stability prototype
     */
    function TemplateApp(request: Request): Object {
        let app = TemplateBuilder(request)
        return app(request)
    }

    /** 
        Template builder. This routine creates a template function to serve requests for template files. 
        The template path can be supplied via the request.filename or a literal template can be provided via options.literal.
        @param request Request object. 
        @param options Object hash of options
        @options layout Path Layout file
        @options literal String containing the template to render. In this case request.filename is ignored.
        @options dir Path Base directory to use for including files and for resolving layout directives
        @return A web script function that services a web request.
        @spec ejs
        @stability prototype
     */
    function TemplateBuilder(request: Request, options: Object = {}): Function {
        let path
        if (!options.literal) {
            path = request.filename
            if (path && !path.exists) {
                request.writeError(Http.NotFound, "Cannot find " + path)
                //  MOB - is this a generic need for a function like this?
                return function() {}
            }
        }
        return Loader.load(path, path, request.config, function (id, path) {
            if (!global.TemplateParser) {
                global.load("ejs.template.mod", {reload: false})
            }
            let data = options.literal || TemplateParser().build(path.readString(), options)
            return Loader.wrap(path, data)
        }).app
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
