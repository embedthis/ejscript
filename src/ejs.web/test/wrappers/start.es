#!/usr/bin/env ejs

require ejs.web
const HTTP = App.config.uris.http

let server: HttpServer = new HttpServer({documents: "web"})
server.on("readable", function (event, request) {
    server.process(Loader.require("web/dispatch.es").app, request)
})
server.listen(HTTP)
App.run()

