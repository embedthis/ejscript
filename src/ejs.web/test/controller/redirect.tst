/*
    Test Controller.redirect*
 */
require ejs.web

const HTTP = App.config.uris.http

public class TestController extends Controller {
    use namespace action

    action function index() {
        redirect("/test/list")
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

//  redirect()
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/index")
http.wait()
assert(http.response == "Hello List")
http.close()

//  redirectAction()
let http = new Http
http.followRedirects = true
http.get(HTTP + "/test/phase1")
http.wait()
assert(http.response == "Hello Phase 2")
http.close()

server.close()
