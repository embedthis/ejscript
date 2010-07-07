/*
    Appweb startup script for ejscript requests 
 */
require ejs.web

let server: HttpServer = new HttpServer(".", "web")

server.observe("readable", function (event, request) {
    Web.serve(request)
})
server.attach()

