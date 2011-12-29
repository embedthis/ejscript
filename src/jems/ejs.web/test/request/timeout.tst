/*
    Test timeouts and Request.finalize, dontAutoFinalize
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    let l = limits

    switch (pathInfo) {
    case "/dontFinalize":
        //  Test an inactivity timeout
        setLimits({inactivityTimeout: 1})
        dontAutoFinalize()
        // This will have no effect
        autoFinalize()
        // Should timeout
        break;

    case "/force":
        //  Test an inactivity timeout
        setLimits({inactivityTimeout: 1})
        dontAutoFinalize()
        finalize()
        // Should NOT timeout
        break;

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

//  Test inactivityTimeout
let http = fetch(HTTP + "/dontFinalize", Http.RequestTimeout)
assert(http.status == Http.RequestTimeout)
assert(http.response.contains("Exceeded inactivity timeout of 1 sec"))

http.close()

//  Test inactivityTimeout
let http = fetch(HTTP + "/force")
assert(http.status == Http.Ok)
assert(http.response == "")
http.close()

server.close()
