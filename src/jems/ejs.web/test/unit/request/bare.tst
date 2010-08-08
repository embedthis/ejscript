/*
    Test bare Request object outside a web server
 */

require ejs.web

r = new Request
assert(r)


assert(r.absHome == null)
assert(r.authGroup == null)
assert(r.authType == null)
assert(r.authUser == null)
assert(r.autoFinalize)
assert(r.contentLength == 0)
assert(r.contentType == null)
assert(r.cookies == null)
assert(r.errorMessage == null)
assert(r.files == null)
assert(r.filename == null)
assert(r.headers && Object.getOwnPropertyCount(r.headers) == 0)
assert(r.home == "/")
assert(r.host == "localhost")
assert(!r.isSecure)
assert(r.limits && Object.getOwnPropertyCount(r.limits) == 0)
assert(r.localAddress == null)
assert(r.method == null)
assert(r.originalMethod == null)
assert(r.originalUri == null)
assert(r.params && Object.getOwnPropertyCount(r.params) == 0)
assert(r.port == null)
assert(r.protocol == null)
assert(r.query == null)
assert(r.reference == null)
assert(r.referrer == null)
assert(r.remoteAddress == null)
assert(r.responseHeaders && Object.getOwnPropertyCount(r.responseHeaders) == 0)
assert(r.route == null)
assert(r.scheme == "http")
assert(r.sessionID == null)
assert(r.status == null)
assert(r.uri == null)
assert(r.env && Object.getOwnPropertyCount(r.env) == 0)

//  Request.config
let c = r.config

assert(c.log)
assert(c.cache)
assert(c.directories)
assert(c.init)
assert(c.test)
assert(c.session)
assert(c.web)
