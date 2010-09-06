/*
    Test Controller database init
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

load("../utils.es")

public class TestController extends Controller {
    use namespace action

    action function none() {
        /* No response, autoFinalize, just set responded */
        assert(request.status == Http.Ok)
        assert(request.responded == false)
        request.responded = true
        assert(request.autoFinalizing)
    }

    action function empty() {
        /* Empty write, autoFinalize, just set responded */
        assert(request.status == Http.Ok)
        assert(request.responded == false)
        write("")
        assert(request.responded)
        assert(request.autoFinalizing)
    }

    action function status() {
        /* Empty write, autoFinalize, just set responded */
        assert(request.status == Http.Ok)
        assert(request.responded == false)
        assert(request.autoFinalizing)
        setStatus(Http.Created)
        assert(request.responded)
        finalize()
    }
} 

let server = new HttpServer
server.on("readable", function (event, request: Request) {
    let [,params.controller, params.action] = pathInfo.toString().split("/")
    let app = Controller.create(request).app
    assert(app is Function)
    Web.process(app, request)
})
server.listen(HTTP)


//  Test autoFinalization with no response
let http = fetch(HTTP + "/test/none")
assert(http.status == Http.Ok)
assert(http.response == "")
http.close()


//  Test autoFinalization with empty response
let http = fetch(HTTP + "/test/empty")
assert(http.status == Http.Ok)
assert(http.response == "")
// http.close()


//  Test autoFinalization with just setting status and no response
let http = fetch(HTTP + "/test/status", Http.Created)
assert(http.response == "")
http.close()

server.close()
