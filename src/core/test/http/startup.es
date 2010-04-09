/*
    Appweb startup script for ejscript requests 
 */
require ejs.web

let server: HttpServer = new HttpServer(".", "web")

server.addListener("readable", function (event, request) {
    Web.serve(request)
})
server.attach()

