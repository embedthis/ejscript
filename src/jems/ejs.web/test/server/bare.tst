/*
    Test bare HttpServer 
 */
require ejs.web

server = new HttpServer
assert(server.documents == ".")
assert(server.home == ".")
assert(server.address == null)
assert(server.async == true)
assert(server.isSecure == false)
assert(server.limits)
assert(server.name == null)
assert(server.port == 0)
assert(server.software.startsWith("ejs-http/"))

//  name
server.name = "example.com"
assert(server.name == "example.com")
