/*
    Router location definitions
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
load("helper.es")
server = new HttpServer
server.listen(HTTP)

//  Server - route request and serialize the request object as a response
server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
    } catch (e) {
        print(e)
        finalize()
        assert(!e)
    }
    write(serialize(request) + "\n")
    finalize()
})


//  Create the router
let router = new Router(null)

//  Define a location block

router.reset()
router.add("/admin", {location: {scriptName: "/manage", dir: "sub"}})
let response = test("/admin")
assert(response.scriptName == "/manage")
assert(Object.getOwnPropertyCount(response.params) == 0)

server.close()
