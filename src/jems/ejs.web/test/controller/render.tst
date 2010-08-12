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
    action function response() {
        return {status: Http.Ok, headers: { Weather: "raining" }, body: "Hello Response" }
    }
} 

load("../utils.es")
server = controllerServer(HTTP)


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


//  renderPartialTemplate
let http = fetch(HTTP + "/test/partial")
assert(http.response == "part-1\npart-2\n")


//  render a response hash
let http = fetch(HTTP + "/test/response")
assert(http.header("Weather") == "raining")
assert(http.response == "Hello Response")


http.close()
server.close()
