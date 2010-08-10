/*
    Test basic controller use
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

public class TestController extends Controller {
    use namespace action

    action function echo() {
        // print(serialize(request, {pretty:true, depth: 2}))
// dump(request)
        assert(actionName == "echo")
        assert(controllerName == "Test")
        assert(params.color == "red")
        assert(params.temp == "cold")
        assert(request.query == "color=red&temp=cold")
        render("Echo Hello World")
    }
} 

server = new HttpServer
server.observe("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/test/echo":
        //  Split the url and 
        let [,params.controller, params.action] = pathInfo.toString().split("/")
        assert(params.controller == "test")
        assert(params.action == "echo")
        let app = Controller.create(request).app
        assert(app is Function)
        Web.process(app, request)
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

server.listen(HTTP)
load("../utils.es")

//  Simple echo action
let http = fetch(HTTP + "/test/echo?color=red&temp=cold")
assert(http.status == Http.Ok)
assert(http.response == "Echo Hello World")

http.close()
server.close()
