/*
    Start http server
*/
require ejs.web

let HTTP = ":" + (App.config.test.http_port || "6700")
let HTTPS = ":" + (App.config.test.ssl_port || "6743")

//  Start regular server
let server: HttpServer = new HttpServer("web")
let router: Router = new Router(Router.Top)

server.on("readable", function (event, request) {
    App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})
App.log.info("Listen on ", HTTP)
server.listen(HTTP)

//  Start secure server
if (Config.SSL) {
    var secureServer: HttpServer = new HttpServer("web")
    secureServer.on("readable", function (event, request) {
        // App.log.info(request.method, request.uri, request.scheme)
        Web.serve(request, router)
    })
    secureServer.secure("ssl/server.key.pem", "ssl/server.crt")
    App.log.info("Secure listen on ", HTTPS)
    secureServer.listen(HTTPS)
}
App.eventLoop()
