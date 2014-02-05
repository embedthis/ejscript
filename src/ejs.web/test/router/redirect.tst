/*
    Router redirect rules
 */
require ejs.web

const HTTP = App.config.uris.http

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


let router = new Router(null)

//  Rewrite the pathInfo.

router.reset()
router.add("/old-path", {redirect: "/new-path"})
router.add("/new-path", { params: { extra: "hello new path" }})
let response = test("/old-path")
assert(response.pathInfo == "/new-path")
assert(Object.getOwnPropertyCount(response.params) == 1)
assert(response.params.extra == "hello new path")

server.close()
