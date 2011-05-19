#!/usr/bin/env ejs
/*
    Just for quick testing - not used by test sub-directories
 */
require ejs.web

let address = App.args[1] || App.config.test.http_port || ":6700"
let server: HttpServer = new HttpServer({documents: "../web"})

var r = new Router(Router.Top, {threaded: true})
// r.show()

server.on("readable", function (event, request) {
    // request.setLimits({timeout: 0, inactivityTimeout: 60})
    // App.log.info(request.method, request.uri)
    server.serve(request, r)
})

App.log.info("Listen on " + address)
server.listen(address)
App.run()
