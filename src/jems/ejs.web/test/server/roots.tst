/*
    Test document and server root
 */
require ejs.web

server = new HttpServer
assert(server.documents == ".")
server.documents = "web"
assert(server.documents == "web")

assert(server.home == ".")
server.home = "/home/example"
assert(server.home == "/home/example")
