/*
    Test HttpServer session limits and timeouts
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || 6700)
load("../utils.es")

server = new HttpServer

server.on("readable", function (event, request) {
    let old = session["value"]
    session["value"] = 42
    write(old)
    finalize()
})


/*
    Can't really test this as the session timer runs once per minute
    //  Set max sessions to 2 and a timeout of 1 second
    assert(server.sessions == null)
    server.setLimits({ sessions: 2, sessionTimeout: 1 })
*/
let base = Session.length

server.listen(HTTP)

//  Fetch and create a new session object per request
http = fetch(HTTP + "/")
assert(http.response == "")
let cookie = http.sessionCookie
assert(cookie.contains("-ejs-session-="))
http.close()


//  Use cookie and utilize the same session
http = new Http
http.setCookie(cookie)
http.get(HTTP + "/")
http.wait(30 * 1000)
assert(http.response == "42")
http.close()


//  Without cookie, a new session will be used
http = new Http
http.get(HTTP + "/")
http.wait(30 * 1000)
assert(http.response == "")
http.close()


server.close()

