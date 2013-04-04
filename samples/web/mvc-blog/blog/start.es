
require ejs.web
    
let server: HttpServer = new HttpServer(".", "web")
var router = Router(Router.TopRoutes)

server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri, request.scheme)
    Web.serve(request, router)
})

//  See ejsrc to modify the endpoint
App.log.info("Listen on", App.config.web.endpoint)
server.listen(App.config.web.endpoint)
App.eventLoop()
