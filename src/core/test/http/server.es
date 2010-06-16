/*
    Test http server
*/
require ejs.web

let address = App.args[1]
let server: HttpServer = new HttpServer(".", "web")

var router = Router(Router.TopRoutes)
server.addListener("readable", function (event, request) {
    // App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})

// print("Listen on " + address)
server.listen(address)

App.eventLoop()
