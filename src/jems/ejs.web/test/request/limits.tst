/*
    Test limits
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    let l = limits

    switch (pathInfo) {
    case "/init":
        assert(l)
        assert(l.chunk > 1024)
        assert(l.connReuse == 200)
        assert(l.inactivityTimeout >= 30)
        assert(l.receive == 256 * 1024)
        assert(l.requestTimeout == 300)
        assert(l.sessionTimeout >= 300)
        assert(l.transmission == 2147483647)
        assert(l.upload == 2147483647)
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
        // No finalize to provoke timeout
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
http.close()


//  Test receive limit
let http = new Http
http.post(HTTP + "/receive", "Too much receive data")
http.finalize()
http.wait()
assert(http.status == Http.EntityTooLarge)
http.close()


//  Test inactivityTimeout
let http = fetch(HTTP + "/inactivity", Http.RequestTimeout)
assert(http.status == Http.RequestTimeout)
http.close()

server.close()
