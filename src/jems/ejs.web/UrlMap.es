/*
    UrlMap.es - Simple Url Router
 */

module ejs.web {

    /** 
        Directory content handler. This redirects requests for directories and serves directory index files.
        If the request pathInfo ends with "/", the request is transparently redirected to an index file if one is present.
        The set of index files is defined by HttpServer.indicies. If the request is a directory but does not end in "/",
        the client is redirected to a URL equal to the pathInfo with a "/" appended.
        @param map Map of URLs to use for routing. TODO
        @param options TODO
        @returns A response hash object
        @spec ejs
        @stability prototype
     */
    function UrlMap(map, options): Object {
        var options ||= { longestMatchFirst: true }
        var mappings = []
        for (var location in map) {
            var app = map[location],
                host = null,
                match
            
            if (match = location.match(/^https?:\/\/(.*?)(\/.*)/)) {
                host = match[1]
                location = match[2]
            }
            if (location.charAt(0) != "/") {
                throw new Error("paths need to start with / (was: " + location + ")")
            }
            mapping.push([host, location.replace(/\/+$/,""), app])
        }
        if (options.longestMatchFirst) {
            mapping = mapping.sort(function(a, b) {
                return (b[1].length - a[1].length) || ((b[0]||"").length - (a[0]||"").length)
            })
        }
        return function(env) {
            var path  = env["PATH_INFO"] ? env["PATH_INFO"].replace(/\/+$/,"") : "",
                hHost = env['HTTP_HOST'], sName = env['SERVER_NAME'], sPort = env['SERVER_PORT']

            for (var i = 0; i < mapping.length; i++) {
                var host = mapping[i][0], location = mapping[i][1], app = mapping[i][2]
                if ((host === hHost || host === sName || (host === null )) &&
                    (location === path.substring(0, location.length)) &&
                    (path.charAt(location.length) === "" || path.charAt(location.length) === "/")) {
                    env = Object.create(env)
                    env["SCRIPT_NAME"] += location
                    env["PATH_INFO"]    = path.substring(location.length)

                    return app(env)
                }
            }
            return exports.notFound(env)
        }
    }
/*
    exports.notFound = function (env) {
        return utils.responseForStatus(404, env.PATH_INFO)
    }
*/
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.md distributed with 
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
