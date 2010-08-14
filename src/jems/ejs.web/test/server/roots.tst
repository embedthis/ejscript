/*
    Test document and server root
 */
require ejs.web

server = new HttpServer
assert(server.documentRoot == ".")
server.documentRoot = "web"
assert(server.documentRoot == "web")

assert(server.serverRoot == ".")
server.serverRoot = "/home/example"
assert(server.serverRoot == "/home/example")
