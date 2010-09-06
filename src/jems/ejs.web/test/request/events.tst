/*
    Test events
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

var events

server.on("readable", function (event, request: Request) {
    on(["close", "error", "readable", "writable"], function(event) {
        // print("GOT " + event)
        events[event] = true
    })
    dontAutoFinalize()
    switch (pathInfo) {
    case "/success":
        //  Won't get a writable event because the writing is complete here
        finalize()
        break

    case "/delayed-finalize":
        // Delay the finalize to enable a writable event 
        Timer(200, function() { finalize() }).start()
        break

    case "/post":
        Timer(200, function() { finalize() }).start()
        break

    case "/error":
        //  The finalize will fail due to a lost connection 
        Timer(200, function() { try { finalize(); } catch {}; }).start()
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

//  Simple 
events = {}
let http = fetch(HTTP + "/success")
//  No writable event because it completed in the callback
assert(events.close)
assert(!(events.error && events.readable && events.writable))
http.close()


//  Delayed-finalize 
events = {}
let http = fetch(HTTP + "/delayed-finalize")
for (i = 0; i < 1000 && !events.close; i++) App.eventLoop(10, 1)
assert(events.close && events.writable)
assert(!(events.error && events.readable))
http.close()


//  Post
events = {}
let http = new Http
http.post(HTTP + "/post", "Some Data")
http.wait()
for (i = 0; i < 1000 && !events.close; i++) App.eventLoop(10, 1)
assert(events.close && events.readable && events.writable)
assert(!(events.error))
http.close()


//  Error
events = {}
let http = new Http
http.post(HTTP + "/error")
http.write("Some")
http.flush()
//  close without finalization
http.close()
for (i = 0; i < 1000 && !events.close; i++) App.eventLoop(10, 1)
assert(events.close && events.error && events.readable && events.writable)
http.close()

server.close()
