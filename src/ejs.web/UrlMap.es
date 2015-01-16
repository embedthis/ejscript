/*
    UrlMap.es - Simple Url Router
 */

# Config.WEB
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
