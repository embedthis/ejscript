/*
    Test Controller.redirect*
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function index() {
print("BEFORE")
        redirect("/test/list")
print("AFTER")
    }
    action function list() {
        write("Hello List")
    }
    action function phase1() {
        redirect("phase2")
    }
    action function phase2() {
        write("Hello Phase 2")
    }
} 

load("../utils.es")
server = controllerServer(HTTP)

print("AA")

//  redirect()
let http = new Http
http.followRedirects = true
print(HTTP + "/test/index")
http.get(HTTP + "/test/index")
http.wait()
assert(http.response == "Hello List")
http.close()

print("BB")

//  redirectAction()
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/phase1")
http.wait()
assert(http.response == "Hello Phase 2")
http.close()

server.close()
