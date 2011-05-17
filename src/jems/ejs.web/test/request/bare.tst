/*
    Test bare Request object outside a web server
 */

require ejs.web

r = new Request("http://example.com:7777/app/index.html#section-1?a=b&c=d")
assert(r)

assert(r.absHome == null)
assert(r.authGroup == null)
assert(r.authType == null)
assert(r.authUser == null)
assert(r.autoFinalizing)
assert(r.contentLength == 0)
assert(r.contentType == null)
assert(r.cookies == null)
assert(r.dir == ".")
assert(r.env && Object.getOwnPropertyCount(r.env) == 0)
assert(r.errorMessage == null)
assert(r.files == null)
assert(r.filename == "app/index.html")
assert(r.headers && Object.getOwnPropertyCount(r.headers) == 0)
assert(r.home == null)
assert(r.host == "example.com")
assert(!r.isSecure)
assert(r.limits && Object.getOwnPropertyCount(r.limits) == 0)
assert(r.localAddress == null)
assert(r.method == null)
assert(r.originalMethod == null)
assert(r.originalUri == null)
assert(r.params && Object.getOwnPropertyCount(r.params) == 0)
assert(r.port == 7777)
assert(r.protocol == null)
assert(r.query == "a=b&c=d")
assert(r.reference == "section-1")
assert(r.referrer == null)
assert(r.remoteAddress == null)
assert(r.responseHeaders && Object.getOwnPropertyCount(r.responseHeaders) == 0)
assert(r.route == null)
assert(r.scheme == "http")
assert(r.sessionID == null)
assert(r.status == null)
assert(r.uri == "http://example.com:7777/app/index.html#section-1?a=b&c=d")

//  Request.config
let c = r.config
assert(c.log)
assert(c.web.cache)
assert(c.dirs)
assert(c.init)
assert(c.test)
assert(c.web)
