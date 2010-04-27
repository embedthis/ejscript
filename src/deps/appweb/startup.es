/*
    Startup script for ejsweb
 */
require ejs.web

// let server: HttpServer = new HttpServer(".", "web")
let server: HttpServer = new HttpServer(null, null)
server.addListener("readable", function (event, request) {
    Web.serve(request)
})
server.attach()

