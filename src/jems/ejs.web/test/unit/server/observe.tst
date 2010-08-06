/*
    Test HttpServer.observe and events
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || 6700)
load("../utils.es")

server = new HttpServer
assert(server.address == null)

events = {}
server.observe("readable", function (event, request) {
    events[event] = true
    request.session
    request.destroySession()
    request.finalize()
})
server.observe(["close", "createSession", "destroySession", "readable"], function (event, request) {
    //  Respond to request and trigger session events. Save record of events.
    events[event] = true
})

//  Run server and fetch one request
assert(server.sessions == null)
server.listen(HTTP)
http = fetch(HTTP)

//  Sessions should now exist with one sesson object
assert(server.sessions)
assert(Object.getOwnPropertyCount(server.sessions) == 1)

//  Close server and verify all events received
server.close()
assert(events.readable)
assert(events.close)
assert(events.createSession)
assert(events.destroySession)

