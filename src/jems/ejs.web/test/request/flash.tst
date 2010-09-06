/*
    Test flash messages and Request.notify()  
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    request.setupFlash()
    switch (pathInfo) {
    case "/first":
        notify("forecast", "rain")
        assert(flash["forecast"] == "rain")
        break

    case "/second":
        assert(flash["forecast"] == "rain")
        break

    case "/third":
        assert(flash["forecast"] == undefined)
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
    request.finalizeFlash()
    finalize()
})

//  First page - set a flash message and return a session cookie
let http = fetch(HTTP + "/first")
let cookie = http.sessionCookie
assert(cookie.contains("-ejs-session-="))
http.close()

//  Second page - test flash message from first page and clear flash
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/second")
http.wait()
http.close()

//  Third page - verify flash is empty
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/third")
http.wait()
http.close()

server.close()
