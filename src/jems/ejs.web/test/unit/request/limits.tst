/*
    Test limits
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.observe("readable", function (event, request: Request) {
    let l = limits

    switch (pathInfo) {
    case "/init":
        assert(l)
        assert(l.chunk > 1024)
        assert(l.receive > 1024 * 1024)
        assert(l.reuse == 100)
        assert(l.transmission > 1024 * 1024)
        assert(l.upload > 1024 * 1024)
        assert(l.inactivityTimeout == 60)
        assert(l.requestTimeout == 0)
        assert(l.sessionTimeout == 3600)
        assert(l.chunk > 0)
        finalize()
        break

    case "/transmission":
        //  Test the max transmission
        setLimits({transmission: 10})
        assert(l.transmission == 10)
        write("Too much data to receive")
        finalize()
        break

    case "/receive":
        //  Test the max receive
        setLimits({receive: 10})
        assert(l.receive == 10)
        break

    case "/inactivity":
        //  Test the inactivity timeout
        setLimits({inactivityTimeout: 1})
        assert(l.inactivityTimeout == 1)
        /* No finalize to provoke timeout */
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
        break
    }
})

//  Test initialization of limits
let http = fetch(HTTP + "/init")
assert(http.response == "")
http.close()


//  Test transmission limit
let http = fetch(HTTP + "/transmission", Http.EntityTooLarge)
assert(http.status == Http.EntityTooLarge)
assert(http.response.contains("Exceeded transmission max body of 10 bytes"))
http.close()

//  Test receive limit
let http = new Http
http.post(HTTP + "/receive", "Too much receive data")
http.finalize()
do { App.eventLoop(10, true) } while(!http.wait())
assert(http.status == Http.EntityTooLarge)
assert(http.response.contains("Request content body of 21 bytes is too big. Limit 10."))
http.close()

//  Test inactivityTimeout
// GC.enabled = false
let http = fetch(HTTP + "/inactivity", Http.RequestTimeout)
assert(http.status == Http.RequestTimeout)
assert(http.response.contains("Inactive request timed out, exceeded inactivity timeout 1 sec"))
http.close()

server.close()
