/*
    Start http server
*/
require ejs.web

let address = App.args[1]
let server: HttpServer = new HttpServer(".", "../web")

var router = Router(Router.TopRoutes)
server.observe("readable", function (event, request) {
    // App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})

// App.log.info("Listen on " + address)
server.listen(address)
App.eventLoop()
