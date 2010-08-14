/*
    Wait for a request to complete
 */
require ejs.web

const WAIT_TIMEOUT = 30000

public function fetch(url: String, status: Number = Http.Ok): Http {
    let http = new Http
    http.get(url)
    http.wait(WAIT_TIMEOUT)
    if (http.status != status) {
        App.log.debug(0, "STATUS is " + http.status + " expected " + status)
        App.log.debug(0, "RESPONSE is " + http.response)
        assert(http.status == status)
    }
    return http
}


/*
    Start a HttpServer that supports controllers and actions. The controller name is ignored.
    URIs are of the form: /controller/action
 */
public function controllerServer(endpoint): HttpServer {
    server = new HttpServer
    server.observe("readable", function (event, request: Request) {
        let [,params.controller, params.action] = pathInfo.toString().split("/")
        let app = Controller.create(request).app
        assert(app is Function)
        Web.process(app, request)
    })
    server.listen(endpoint)
    return server
}

