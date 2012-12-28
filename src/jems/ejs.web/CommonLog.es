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
                Sample:  10.0.0.5 - - [16/Mar/2013:15:40:36 -0700] "GET /index.html HTTP/1.1" 200 44
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

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
