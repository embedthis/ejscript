/*
    Start http server
*/
require ejs.web

let address = ":" + (App.config.test.ssl_port || "6743")
let server: HttpServer = new HttpServer({documents: "web"})

/* OK
//  TODO -- test protocols and ciphers
    server.secure("ssl/server.key.pem", "ssl/server.crt")
    print("IS SECURE " + server.isSecure)
    // server.verifyClients("ssl/ca", "ssl/ca/file.certs")


    // server.name = "embedthis.com"

var router = Router(Router.Default)
server.on("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    server.serve(request, router)
})

App.log.info("Listen on " + address)
server.listen(address)

    print("SW " + server.software)
*/

App.run()
