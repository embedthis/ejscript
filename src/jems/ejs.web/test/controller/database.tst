/*
    Test Controller database init
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

let events = {}
load("../utils.es")

public class TestController extends Controller {
    use namespace action
    action function list() {
        events["list"] = true
        write("Hello")
    }
} 

public class MyDb {
    function MyDb() {
        events["constructor"] = true
    }
}

let server = new HttpServer
server.on("readable", function (event, request: Request) {
    let [,params.controller, params.action] = pathInfo.toString().split("/")
    request.config.mode = "debug",
    request.config.database = {
        "class": "MyDb",
        "debug": { name: "anydb" },
    }
    let app = Controller.create(request).app
    assert(app is Function)
    Web.process(app, request)
})
server.listen(HTTP)


let http = fetch(HTTP + "/test/list")
assert(http.status == Http.Ok)
assert(http.response == "Hello")
http.close()

server.close()
