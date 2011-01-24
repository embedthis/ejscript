/*
    Test HttpServer.on and events
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || 6700)
load("../utils.es")

events = {}
server = new HttpServer
assert(server.address == null)

/*
    Listen for incoming Http requests
 */
server.on("readable", function (event, request) {
    events[event] = true
    request.session
    request.destroySession()
    request.finalize()
})

/*
    Listen for server events of interest. Save record of events.
 */
function watch(event, request) {
    events[event] = true
}
server.on(["close", "createSession", "destroySession", "readable"], watch)

//  Run server and fetch one request
assert(server.sessions == null)
server.listen(HTTP)
http = fetch(HTTP + "/")

//  Sessions should now exist with one sesson object
assert(server.sessions)
assert(Object.getOwnPropertyCount(server.sessions) == 1)

//  Close server and verify all events received
server.close()
server.off(["close", "createSession", "destroySession", "readable"], watch)
assert(events.close)
assert(events.readable)
assert(events.createSession)
assert(events.destroySession)
