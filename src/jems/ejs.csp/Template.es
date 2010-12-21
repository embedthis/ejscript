/*
    Template.es -- Ejscript web templating loader. 
 */

module ejs.ctemplate  {

    /** Web Page Template. This parses an web page with embedded Ejscript directives.
      
        The template engine provides  embedded Ejscript using <% %> directives. It supports:
      
          <%                    Begin an ejs directive section containing statements
          <%=                   Begin an ejs directive section that contains an expression to evaluate and substitute
          %>                    End an ejs directive
          <%@ include "file" %> Include an ejs file
          <%@ layout "file" %>  Specify a layout page to use. Use layout "" to disable layout management.
      
        Directives for use outside of <% %> 
          @@var                 To expand the value of "var". Var can also be simple expressions (without spaces).
      
        TODO implement these directives
          -%>                   Omit newline after tag
          <%h                   Html escape the output

        @stability prototype
        @spec ejs
     */
    public class Template {
        use default namespace public

        /** Load a templated web page.
            @param request Web Request object
            @returns An exports object with a function "app" property representing the web page
         */
        static function load(request: Request): Object {
            let path = request.dir.join(request.pathInfo.slice(1))
            Loader.setConfig(request.config)
            return Loader.load(path, path, function (path) {
                let data = TemplateParser().build(path.readString())
                return Loader.wrap(data)
            })
        }
    }
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.
  
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
