#!/usr/bin/env ejs
/*
    Just for quick testing - not used by test sub-directories
 */
require ejs.web

const HTTP = App.args[1] || App.config.uris.http
let server: HttpServer = new HttpServer({documents: "../web"})

var r = new Router(Router.Top, {workers: true})
// r.show("full")

server.on("readable", function (event, request) {
    //  request.setLimits({timeout: 0, inactivityTimeout: 60})
    //  App.log.info(request.method, request.uri)
    //  BUG When multithreaded, "r" is not visible in the worker
    server.serve(request, r)
})

App.log.info("Listen on " + HTTP)
server.listen(HTTP)
App.run()
