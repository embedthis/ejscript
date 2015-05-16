#!/usr/bin/env ejs

/*
    Start http server
*/
require ejs.web

const HTTP = App.config.uris.http
const HTTPS = App.config.uris.ssl

//  Start regular server
let server: HttpServer = new HttpServer({documents: "web"})
let router: Router = new Router(Router.Top)

router.show()
server.on("readable", function (event, request) {
    App.log.info(request.method, request.uri, request.scheme)
    server.serve(request, router)
})
server.listen(HTTP)

//  Start secure server
if (Config.SSL) {
    var secureServer: HttpServer = new HttpServer({documents: "web"})
    secureServer.on("readable", function (event, request) {
        secureServer.serve(request, router)
    })
    secureServer.secure("../../../certs/self", "../../../certs/self.crt")
    secureServer.listen(HTTPS)
}
App.run()
