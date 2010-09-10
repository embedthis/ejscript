/*
    Router redirect rules
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


//  Create the router and set the default buidler
let router = new Router
router.setDefaultBuilder(builder)


//  Rewrite the pathInfo.

router.reset()
router.add("/old-path", {redirect: "/new-path"})
router.add("/new-path", { params: { extra: "hello new path" }})
let response = test("/old-path")
assert(response.pathInfo == "/new-path")
assert(Object.getOwnPropertyCount(response.params) == 1)
assert(response.params.extra == "hello new path")

server.close()
