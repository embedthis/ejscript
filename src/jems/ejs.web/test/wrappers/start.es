#!/usr/bin/env ejs

require ejs.web
var address = ":" + (App.config.test.http_port || "6700")

let server: HttpServer = new HttpServer("web")
server.on("readable", function (event, request) {
    Web.process(Loader.require("web/dispatch.es").app, request)
})
server.listen(address)
App.run()

