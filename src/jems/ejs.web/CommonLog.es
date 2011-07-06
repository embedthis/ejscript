/*
    CommonLog.es -- Common Log Format logger.
 */

module ejs.web {
    /** 
        Common Log web server logging. 
        This function is a middleware constructor that returns a web application function.
        @param app Application servicing the request and generating the response.
        @param logger Stream to use for writing access log information
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = CommonLog(app)
        @spec ejs
        @stability prototype
     */
    function CommonLog(app, logger: Stream = App.log): Object {
        return (new CommonLogClass(app, logger)).app
    }

    //  TODO - not a great name having Class in the name
    /**
        Common Log web server logging.
        This logs each HTTP request to a file in the Common Log format defined by the Apache web server.
     */
    class CommonLogClass {
        var innerApp: Function
        var logger: Stream

        /**
            Constructor for the CommonLogClass
            @param app Application function
            @param logger Optional logger. Defaults to App.log
         */
        function CommonLogClass(app, logger: Stream = App.log) {
            this.innerApp = app
            this.logger = logger
        }

        //  MOB -- rename
        function app(request: Request): Object {
            let start = new Date
            let response = innerApp.call(request, request)
            let size = (response.body is String) ? response.body.length : 0
            /*
                Sample:  10.0.0.5 - - [16/Mar/2011:15:40:36 -0700] "GET /index.html HTTP/1.1" 200 44
             */
            let user = request.authUser || "-"
            let uri = request.pathInfo + (request.query ? ("?" + request.query) : "")
            logger.write(request.remoteAddress + ' - ' + user + ' [' + Date().format("%d/%b/%Y %T %Z") + 
                '] "' + request.method + ' ' + uri + ' ' + request.protocol + '" ' + response.status + ' ' + 
                size + ' ' + start.elapsed + "\n")
            return response
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Portions Copyright (c) 2009 Thomas Robinson 280north.com (http://280north.com/)
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
    vim: sw=4 ts=4 expandtab

    @end
 */
