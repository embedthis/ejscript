/*
    HttpServer unit tests
*/

/*
    Questions
    - What is serverRoot for?
    - What is documentRoot for?

- Test sync server
- Test async server
- SSL
- DocumentRoot, serverRoot
- Appweb unit test listen/attach
- ejsrc
*/

require ejs.web

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)
const HTTPS = "https://127.0.0.1:" + ((global.test && test.config.ssl_port) || 6743)


/*OK
//  Construction
server = new HttpServer
assert(server)
assert(server.documentRoot == ".")
assert(server.serverRoot == ".")
assert(server.async)
assert(server.port == 0)
assert(server.software.contains("ejs-http"))


//  serverRoot and documentRoot
server = new HttpServer("sr", "dr")
assert(server.documentRoot is Path)
assert(server.serverRoot is Path)
assert(server.serverRoot == "sr")
assert(server.documentRoot == "dr")
server.documentRoot = "../web"
assert(server.documentRoot == "../web")
assert(server.name == null)

//  name
server.name = "example.com"
assert(server.name == "example.com")
*/

/*
//  Create Async server
server = new HttpServer(".", "../web")
server.observe("readable", function (event: String, request: Request) {
    request.finalize()
})
App.log.debug(0, "LISTEN on " + ENDPOINT)
server.listen(ENDPOINT)
server.close()


//  Simple access
server = new HttpServer(".", "../web")
server.observe("readable", function (event: String, request: Request) {
    print("THERE")
    request.write("Hello World")
    request.finalize()
})
server.listen(ENDPOINT)

print("HERE")

let client = new Http
client.async = true
client.get(ENDPOINT + "/index.html")
client.finalize()
client.observe("readable", function (event, ) {
})
print(client.readString())
client.close()

App.eventLoop()
*/

/*
server.documentRoot


        native function HttpServer(serverRoot: Path = ".", documentRoot: Path = ".")
        native function observe(name, observer: Function): Void
        native function get address(): String 
        native function get async(): Boolean
        native function set async(enable: Boolean): Void
        native function close(): Void
        var documentRoot: Path
        native function listen(endpoint: String?): Request
        native function secureListen(endpoint: String?, keyFile: Path, certFile: Path, protocols: Array, ciphers: Array): Void
        native function get port(): Number 
        native function removeObserver(name: Object, observer: Function): Void
        var serverRoot: Path
        native function get software(): String
*/
