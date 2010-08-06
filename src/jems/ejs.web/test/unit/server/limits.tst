/*
    Test HttpServer.limits
 */
require ejs.web

server = new HttpServer
let l = server.limits
assert(l.chunk > 2048)
assert(l.receive > 1024 * 1024)
assert(l.reuse == 100)
assert(l.transmission > 1024 * 1024)
assert(l.upload > 1024 * 1024)
assert(l.inactivityTimeout == 60)
assert(l.requestTimeout == 0)
assert(l.sessionTimeout == 3600)
assert(l.clients >= 10)
assert(l.header >= 2048)
assert(l.headers >= 20)
assert(l.requests >= 20)
assert(l.sessions >= 100)
assert(l.stageBuffer >= 4096)
assert(l.uri >= 512)
