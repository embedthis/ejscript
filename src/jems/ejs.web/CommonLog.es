/*
    CommonLog.es -- Common Log Format logger
 */

module ejs.web {
    /** 
        CommonLogger middleware script. This logs each request to a file in the Common Log format defined 
        by the Apache web server.
        @param app Application servicing the request and generating the response.
        @param logger Logger stream
        @return A response function
        @example:
            export.app = CommonLog(app)
     */
    var CommonLogger = function(app, logger) {
        return function(request) {
            return (new CommonLoggerContext(app, logger)).run(request);
        }
    }

    class CommonLoggerContext {
        var app: Function
        var logger
        var request: Request
        var start: Date

        function CommonLoggerContext(app, logger) {
            this.app = app
            this.logger = logger || App.log
        }

        function run(request: Request) {
            this.request = request
            start = new Date
            var result = this.app(request)
            return result
        }

        function log(string) {
            this.request["jsgi.errors"].print(string)
            this.request["jsgi.errors"].flush()
        }

        function forEach(block) {
            var length = 0
            
            this.body.forEach(function(part) {
                length += part.toByteString().length
                block(part)
            })
            logger.write(request.remoteAddress + ' - ' + request.authUser + '[' + Date().format("%d/%b/%Y %T") + 
                ']"' + request.method + ' ' + request.uri + ' ' + request.protocol + ' ' + status + ' ' + 
                size + ' ' + start.elapsed())
            }
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Portions Copyright (c) 2009 Thomas Robinson 280north.com (http://280north.com/)
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
