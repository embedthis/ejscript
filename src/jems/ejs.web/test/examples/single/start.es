require ejs.web

let address = ":" + (App.config.test.http_port || "6700")

// server.create(address, ".")

function app() {
    return {}
}

let server: HttpServer = new HttpServer({documents: "web"})
server.on("readable", function (event, request) {
    // server.process(app)
    server.process(Head(ContentType(app)))
})
server.listen(address)
App.run()
