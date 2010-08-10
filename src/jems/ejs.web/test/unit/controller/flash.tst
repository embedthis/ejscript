/*
    Test Controller.render*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function first() {
        error("one", 1)
        flash("two", 2)
        inform("three", 3)
        warn("four", 4)
        render()
    }
    action function second() {
        assert(request.flashMessages["one"] == 1)
        assert(request.flashMessages["two"] == 2)
        assert(request.flashMessages["three"] == 3)
        assert(request.flashMessages["four"] == 4)
        render()
    }
    action function third() {
        assert(Object.getOwnPropertyCount(request.flashMessages) == 0)
        render()
    }
} 

server = new HttpServer
server.observe("readable", function (event, request: Request) {
    let [,params.controller, params.action] = pathInfo.toString().split("/")
dump(params)
    let app = Controller.create(request).app
    Web.process(app, request)
})

server.listen(HTTP)
load("../utils.es")

//  first
let http = fetch(HTTP + "/test/first")
let cookie = http.sessionCookie
assert(cookie.contains("-ejs-session-="))
http.close()

//  second - test flash messages from first page and clear flash
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/test/second")
do { App.eventLoop(10, true) } while(!http.wait())
http.close()

//  third - verify flash is empty
let http = new Http
http.setCookie(cookie)
http.get(HTTP + "/test/third")
do { App.eventLoop(10, true) } while(!http.wait())
http.close()

server.close()
