/*
    Test Controller.write*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function first() {
        notify("general", "zero")
        error(1)
        inform(2)
        warn(3)
        finalize()
    }
    action function second() {
        assert(request.flash["general"] == "zero")
        assert(request.flash["error"] == 1)
        assert(request.flash["inform"] == 2)
        assert(request.flash["warn"] == 3)
        finalize()
    }
    action function third() {
        assert(Object.getOwnPropertyCount(request.flash) == 0)
        finalize()
    }
} 

load("../utils.es")
server = controllerServer(HTTP)


//  first - Send a request to set flash messages and get a session cookie
let http = fetch(HTTP + "/test/first")
let cookie = http.sessionCookie
assert(cookie.contains("-ejs-session-="))
http.close()


//  second - test flash messages from first page and clear flash
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/test/second")
http.wait()
http.close()


//  third - verify flash is empty
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/test/third")
http.wait()
http.close()

server.close()
