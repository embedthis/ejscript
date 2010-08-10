/*
    Controller calls super in constructor
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

public class TestController extends Controller {
    use namespace action

    function TestController(request: Request) {
        //  Must call super if creating using the constructor form
        super(request)
    }
    action function echo() {
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

        //  Use constructor form instead of create form
        let app = TestController(request).app
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
let http = fetch(HTTP + "/test/echo")
assert(http.status == Http.Ok)
assert(http.response == "Echo Hello World")

http.close()
server.close()
