/*
    Test events


    MOB - issues
        - Long running (timer) script. Should it keep running if the connection is closed?
            - How to do a background script?
        - Bug in chunk parsing code
            - Missing http.finalize (and calling http.close()) causes the client to return and not close because
                the final chunk did not get completed  -- but conn is closed (read == -1) and the socket is not getting 
                closed.
        - Missing error event by calling finalize on a closed socket

    ACTIONS:
        1. http.close should call http.finalize() if required
        2. Consider how to do background / long running scripts
        2. Fix readEvent
        3. 
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

var events

server.on("readable", function (event, request: Request) {

    on(["close", "error", "readable", "writable"], function(event) {
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
        //  The finalize may fail due to a lost connection
//  MOB -- not getting an error event for this
        Timer(200, function() { try { finalize() } catch {} }).start()
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

/**MOB
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
for (i = 0; i < 1000 && !events.close; i++) App.run(10, 1)
assert(events.close && events.writable)
assert(!(events.error && events.readable))
http.close()

//  Post
events = {}
let http = new Http
http.post(HTTP + "/post", "Some Data")
http.wait()
for (i = 0; i < 1000 && !events.close; i++) App.run(10, 1)
assert(events.close && events.readable && events.writable)
assert(!(events.error))
http.close()

**/

//  Error
events = {}
let http = new Http
http.post(HTTP + "/error")
http.write("Some")
http.flush()
http.close()

for (i = 0; i < 10 && !events.close; i++) { App.run(1000, 1); }
assert(events.close && events.readable && events.writable)

server.close()
