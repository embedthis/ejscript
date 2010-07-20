/*
    Start http server
*/
require ejs.web

let HTTP = App.args[1] || ":6700"
let HTTPS = App.args[2] || ":6743"

var router = Router(Router.TopRoutes)

//  Start regular server
let server: HttpServer = new HttpServer(".", "../web")

server.observe("readable", function (event, request) {
    // App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})
// App.log.info("Listen on ", HTTP)
server.listen(HTTP)

//  Start secure server
let secureServer: HttpServer = new HttpServer(".", "../web")
secureServer.observe("readable", function (event, request) {
    // App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})

secureServer.secure("ssl/server.key.pem", "ssl/server.crt")
// App.log.info("Secure listen on ", HTTPS)
secureServer.listen(HTTPS)
App.eventLoop()
