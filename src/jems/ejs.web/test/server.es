/*
    Start http server
*/
require ejs.web

let address = App.args[1] || ":6700"
let server: HttpServer = new HttpServer(".", "web")

/* OK
//  MOB -- test protocols and ciphers
    server.secure("ssl/server.key.pem", "ssl/server.crt")
    print("IS SECURE " + server.isSecure)
*/

//  NEXT
// server.verifyClients("ssl/ca", "ssl/ca/file.certs")


/* OK
    // server.name = "embedthis.com"
*/

var router = Router(Router.TopRoutes)
server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    Web.serve(request, router)
})

App.log.info("Listen on " + address)
server.listen(address)

/*OK
    print("SW " + server.software)
*/

App.eventLoop()
