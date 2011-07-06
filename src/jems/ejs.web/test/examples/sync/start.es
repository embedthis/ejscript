/*
    Sync http server
 */
require ejs.web

let address = ":" + (App.config.test.http_port || "6700")
let server: HttpServer = new HttpServer({documents: "../../web"})
server.async = false
server.listen(address)

/* Blocking server */
while (request = server.accept(address)) {
    // App.log.info(request.method, request.uri, request.scheme)
    request.write("DONE")
    request.finalize()
}
