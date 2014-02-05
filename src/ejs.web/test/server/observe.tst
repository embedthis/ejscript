/*
    Test HttpServer.on and events
 */
require ejs.web

const HTTP = App.config.uris.http
load("../utils.es")

events = {}
server = new HttpServer
assert(server.address == null)

//  Listen for incoming Http requests
server.on("readable", function (event, request) {
    events[event] = true
    request.session
    request.destroySession()
    request.finalize()
})

//  Listen for server events of interest. Save record of events.
function watch(event, request) {
    events[event] = true
}
server.on(["close", "readable"], watch)

//  Run server and fetch one request
server.listen(HTTP)
http = fetch(HTTP + "/")

//  Close server and verify all events received
server.close()

server.off(["close", "readable"], watch)
assert(events.close)
assert(events.readable)
