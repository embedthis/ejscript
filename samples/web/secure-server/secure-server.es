/*
    secure-server.es - Secure server using SSL

    Serves content in the "web" directory

    Usage: ejs secure-server.es [PORT]
    Client:
        http :PORT/index.html
        http :PORT/test.ejs
        http :PORT/hello.ejs
*/
require ejs.web

let address = App.args[1] || ":4443"
let server: HttpServer = new HttpServer(".", "web")

/*
    Configure the server private key and certificate
 */
server.secure("../../../src/certs/self.key", "../../../src/certs/self.crt")

server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    Web.serve(request)
})

App.log.info("Listen on " + address)
server.listen(address)
App.eventLoop()
