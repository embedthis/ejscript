/*
    Ejs script to serve web requests on port
*/
require ejs.web

let server: HttpServer = new HttpServer(".", "web")

var router = Router(Router.TopRoutes)
server.addListener("readable", function (event, request) {
    Web.serve(request, router)
})

server.listen("127.0.0.1:6700")
App.eventLoop()
