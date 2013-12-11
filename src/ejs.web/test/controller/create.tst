/*
    Test Controller.create
 */
require ejs.web

const HTTP = App.config.uris.http

public class TestController extends Controller {
    use namespace action

    action function echo() {
        assert(actionName == "echo")
        assert(controllerName == "Test")
        assert(params.color == "red")
        assert(params.temp == "cold")
        assert(request.query == "color=red&temp=cold")

        assert(config)
        assert(request)
        assert(request.controller == this)
        assert(pathInfo == "/test/echo")
        assert(absHome == HTTP + "/")
        assert(home == "../../")
        assert(uri == HTTP + "/test/echo?color=red&temp=cold")
        write("Echo Hello World")
    }
} 

load("../utils.es")

let server = new HttpServer
server.on("readable", function (event, request: Request) {
    [,params.controller, params.action] = pathInfo.toString().split("/")
    let app = Controller.create(request).app
    assert(app is Function)
    server.process(app, request)
})
server.listen(HTTP)


//  Simple echo action
let http = fetch(HTTP + "/test/echo?color=red&temp=cold")
assert(http.status == Http.Ok)
assert(http.response == "Echo Hello World")
http.close()

server.close()
