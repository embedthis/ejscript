require ejs.web

let address = ":" + (App.config.test.http_port || "6700")

// Web.run(address, ".")

function app() {
    return {}
}

let server: HttpServer = new HttpServer(documentRoot, serverRoot)
server.on("readable", function (event, request) {
    // Web.process(app)
    Web.process(Head(ContentType(app)))
})
server.listen(address)
App.eventLoop()
