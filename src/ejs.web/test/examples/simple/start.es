/*
    Simple http server
 */
require ejs.web

const HTTP = App.args[1] || App.config.uris.http
let server: HttpServer = new HttpServer({documents: "../../web"})

server.on("readable", function (event, request) {
    server.serve(request)
})

server.listen(HTTP)
App.run()
