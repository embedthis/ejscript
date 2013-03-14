#!/usr/bin/env ejs
require ejs.web

const HTTP = App.args[1] || App.config.uris.http
const HTTPS = App.args[2] || App.config.uris.ssl
let server: HttpServer = new HttpServer({documents: "../web"})
let ssl: HttpServer = new HttpServer({documents: "../web"})

var r = new Router(Router.Top)

server.on("readable", function (event, request) {
    server.serve(request, r)
})
ssl.on("readable", function (event, request) {
    ssl.serve(request, r)
})

App.log.info("Listen on " + HTTP)
server.listen(HTTP)

ssl.secure('../sslconf/test.key', '../sslconf/test.crt')
App.log.info("Listen on " + HTTPS)
ssl.listen(HTTPS)
App.run()
