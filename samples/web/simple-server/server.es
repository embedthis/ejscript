/*
    Simple Http server

    Serves content in the "web" directory

    Usage: ejs server.es [PORT]
    Client:
        http :PORT/index.html
        http :PORT/test.ejs
        http :PORT/hello.ejs
 */
require ejs.web

let address = App.args[1] || ":4000"
let server: HttpServer = new HttpServer(".", "web")

/*
    Listen for client requests. This make the server "readable" */
server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    /*
        Route and process the request 
     */
    Web.serve(request)
})

/*
    Listen for client connections and loop forever
 */
App.log.info("Listen on " + address)
server.listen(address)
App.eventLoop()
