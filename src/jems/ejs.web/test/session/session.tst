/*
    Test Session() 
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

load("../utils.es")
server = new HttpServer
server.listen(HTTP)
server.setLimits({ sessions: 10, sessionTimeout: 60 })
server.setLimits({ inactivityTimeout: 0, requestTimeout: 0 })

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/basic":
        assert(sessionID == null)
        //  Demand create session
        session["test"] = 1234
        session["obj"] = { color: "red", temp: "low" }
        assert(sessionID)
        finalize()
        break

    case "/reuse-cookie":
        let old = sessionID
        assert(sessionID)
        createSession()
        assert(old == sessionID)
        assert(session["test"] == 1234)
        obj = session["obj"]
        assert(serialize(obj) == '{"color":"red","temp":"low"}')
        destroySession()
        finalize()
        break

    case "/destroy":
        assert(sessionID == null)
        createSession()
        assert(sessionID)
        destroySession()
        finalize()
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})


//  Create a session and save the session cookie
http = fetch(HTTP + "/basic")
assert(http.status == Http.Ok)
let cookie = http.sessionCookie
assert(cookie.contains("-ejs-session-="))
http.close()


//  Use the same cookie to test access to the session
http.setCookie(cookie)
http.get(HTTP + "/reuse-cookie")
http.finalize()
do { App.run(10, true) } while(!http.wait())
assert(http.status == 200)
http.close()

//  destroySession
http = fetch(HTTP + "/destroy")
http.close()

server.close()
