/*
    Monitor.es - Monitor client, request and session resource consumption. 
 */

module ejs.web {

    /** 
        Monitor resource consumption. This monitors the number of simultaneous requests and unique clients against
        thresholds defined in the Request.server (HttpServer) limits.
        @param app Base web app function object
        @returns A new web application function object
        @spec ejs
        @stability prototype
     */
    function Monitor(app): Function {
print("MONITOR TOP")
        var clients = {}
        var clientCount: Number = 0
        var requestCount: Number = 0

        return function (request: Request): Object {
print("MONITOR RUN")
            let server = request.server
            let limits = server.limits
print("CLIENTS  " + clientCount + "/" + limits.clients)
print("REQUESTS " + requestCount + "/" + limits.requests)
            if (clientCount >= limits.clients) {
                return { status: Http.ServiceUnavailable, body: "Server busy, please try again later." }
            }
            if (requestCount >= limits.requests) {
                return { status: Http.ServiceUnavailable, body: "Server busy, please try again later." }
            }
            requestCount++
            try {
                //  MOB -- does this work multithreaded?
dump("BEFORE", clients)
                if (clients[request.remoteAddress]) {
                    clients[request.remoteAddress]++
                } else {
                    clients[request.remoteAddress] = 1
                    clientCount++
                }
dump("MID", clients)
                response = app(request)
            } finally {
print('FINALLY ' + clients[request.remoteAddress])
                if (--clients[request.remoteAddress] == 0) {
                    delete clients[request.remoteAddress]
                    clientCount--
                }
            }
dump("AFTER", clients)
            requestCount--
            return response
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
