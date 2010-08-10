/*
    Test Controller.render*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function index() {
        redirectAction("list")
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

//  first
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/index")
do { App.eventLoop(10, true) } while(!http.wait())
assert(http.response == "Hello List")
http.close()

server.close()
