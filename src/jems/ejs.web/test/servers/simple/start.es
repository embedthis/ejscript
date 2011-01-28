/*
    Simple http server
 */
require ejs.web

let address = App.args[1] || App.config.test.http_port || ":6700"
let server: HttpServer = new HttpServer("../../web")

server.on("readable", function (event, request) {
    Web.serve(request)
})

server.listen(address)
App.run()
