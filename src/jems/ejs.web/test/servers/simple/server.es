/*
    Test http server
 */
require ejs.web

let address = App.args[1]
let server: HttpServer = new HttpServer(".", "../../web")

var router = Router(Router.TopRoutes)
server.observe("readable", function (event, request) {
    Web.serve(request, router)
})

server.listen(address)
App.eventLoop()
