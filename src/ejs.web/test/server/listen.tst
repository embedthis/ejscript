/*
    Test HttpServer.listen properties
 */
require ejs.web

const HTTP = App.config.uris.http
const PORT = Uri(HTTP).port

server = new HttpServer
assert(server.address == null)

/*
    This causes a firewall warning on macosx
// Listen on all ports
server.listen(":" + PORT)
assert(server.address == "")
assert(server.port == 6700)
assert(server.name == null)
assert(!server.isSecure)
server.close()
*/


// With an IP to set server.address
server = new HttpServer
server.listen("127.0.0.1:" + PORT)
assert(server.address == "127.0.0.1")
assert(server.port == 6700)
assert(server.name == null)
assert(!server.isSecure)
server.close()


// SSL
if (Config.SSL) {
    server = new HttpServer
    server.secure("ssl/server.key.pem", "ssl/server.crt")
    server.listen("127.0.0.1:" + PORT)
    assert(server.isSecure)
    assert(server.address == "127.0.0.1")
    assert(server.port == 6700)
    assert(server.name == null)
    server.close()
}
