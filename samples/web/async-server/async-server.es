/*
    async-server.es - Async logging server

    Usage: ejs async-server [port]

    Client: http --post --data "some data" PORT:/

    This server echoes user data back to the client.

 */
require ejs.web
require ejs.web.template
require ejs.cjs

var address = App.args[1] || ":6700"
var server: HttpServer = new HttpServer

/*
    Wait for connections
 */
server.observe("readable", function (event, request) {
    /*
        Got a new request
     */
    request.observe("readable", function (event, request) {
        /*
            Echo data back to the client
         */
        let data = new ByteArray
        if (request.read(data)) {
            request.write(data)
        } else {
            /* End of input */
            request.status = 200
            request.finalize()
        }
    })
})

server.listen(address)
App.log.info("Listen on " + address)
App.eventLoop()


