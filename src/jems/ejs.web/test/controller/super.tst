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

load("../utils.es")
server = controllerServer(HTTP)


//  Simple echo action
let http = fetch(HTTP + "/test/echo")
assert(http.status == Http.Ok)
assert(http.response == "Echo Hello World")


http.close()
server.close()
