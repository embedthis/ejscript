/*
    Test Controller.create
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function echo() {
        assert(actionName == "echo")
        assert(controllerName == "Test")
        assert(params.color == "red")
        assert(params.temp == "cold")
        assert(request.query == "color=red&temp=cold")

        assert(config)
        assert(!rendered)
        assert(request)
        assert(request.controller == this)
        assert(pathInfo == "/test/echo")
        assert(absHome == "http://127.0.0.1:" + PORT + "/")
        assert(home == "../../")
        assert(uri == "http://localhost:6700/test/echo?color=red&temp=cold")

        render("Echo Hello World")
        assert(rendered)
    }
} 

server = new HttpServer
server.observe("readable", function (event, request: Request) {
    let [,params.controller, params.action] = pathInfo.toString().split("/")
    assert(params.controller == "test")
    assert(params.action == "echo")
    let app = Controller.create(request).app
    assert(app is Function)
    Web.process(app, request)
})

server.listen(HTTP)
load("../utils.es")

//  Simple echo action
let http = fetch(HTTP + "/test/echo?color=red&temp=cold")
assert(http.status == Http.Ok)
assert(http.response == "Echo Hello World")

http.close()
server.close()
