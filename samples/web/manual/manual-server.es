/*
    Manual Http server

    Serves content in the "web" directory and does not use the router or Web class to serve requests

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
    Listen for client requests. This make the server "readable" 
 */
server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    request.config = config
    try {
        router.route(request)
        /*
            Load the app/page as a JSGI exports object
         */
        let exports = Web.load(request)
        if (exports) {
            /*
                Start the JSGI application
             */
            Web.start(request, exports.app)
        }
    } catch (e) {
        request.writeError(e)
    }
})

/*
    Listen for client connections and loop forever
 */
App.log.info("Listen on " + address)
server.listen(address)
App.eventLoop()
