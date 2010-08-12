/*
    Test Controller.render*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function error() {
        renderError(201, "error-msg")
    }
    action function file() {
        renderFile("../utils.es")
    }
    action function index() {
        renderView()
    }
    action function viewFile() {
        renderTemplate("list.ejs")
    }
    action function partial() {
        renderPartialTemplate("web/part1.ejs")
        renderPartialTemplate("web/part2.ejs")
        render()
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

/*****
//  renderError
let http = fetch(HTTP + "/test/error", 201)
assert(http.response.contains("log.showClient"))
assert(http.response.contains("error-msg"))
http.close()

//  renderFile
let http = fetch(HTTP + "/test/file")
assert(Path("../utils.es").readString() == http.response)
http.close()

//  renderView
let http = fetch(HTTP + "/test/index")
assert(http.response == "Hello Index\n")
http.close()

//  renderTemplate
let http = fetch(HTTP + "/test/viewFile")
assert(http.response == "Hello View\n")
http.close()
*/

//  renderPartialTemplate
let http = fetch(HTTP + "/test/partial")
assert(http.response == "part-1\npart-2\n")

http.close()
server.close()
