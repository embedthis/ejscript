/*
    Test Controller.render*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function index() {
        redirect("/test/list")
    }
    action function list() {
        render("Hello List")
    }
    action function phase1() {
        redirectAction("phase2")
    }
    action function phase2() {
        render("Hello Phase 2")
    }
} 

server = new HttpServer
server.observe("readable", function (event, request: Request) {
    let [,params.controller, params.action] = pathInfo.toString().split("/")
    let app = Controller.create(request).app
    Web.process(app, request)
})

server.listen(HTTP)
load("../utils.es")

/*OK
//  redirect()
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/index")
do { App.eventLoop(10, true) } while(!http.wait())
print(http.response)
assert(http.response == "Hello List")
http.close()
*/

//  redirectAction()
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/phase1")
do { App.eventLoop(10, true) } while(!http.wait())
print(http.response)
assert(http.response == "Hello Phase 2")
http.close()

server.close()
