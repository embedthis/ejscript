/*
    Router responses
 */
require ejs.web

const HTTP = App.config.uris.http

load("helper.es")
server = new HttpServer
server.listen(HTTP)

//  Server - route request and serialize the request object as a response
server.on("readable", function (event, request: Request) {
    try {
        let route = router.route(request)
        server.process(route.response, request)
    } catch (e) {
        print(e)
        finalize()
        assert(!e)
    }
    // write(serialize(request) + "\n")
    finalize()
})

let router = new Router(null)


//  Run with inline response

router.reset()
router.add("/custom/test", { response: { status: 200, body: "hello world\n" }})
let response = test("/custom/test")
assert(response == "hello world")


//  Run with function response

router.reset()
router.add("/custom/test", { response: function builder(request) {
    return { body: "Hello Cruel World\n" }
}})
let response = test("/custom/test")
assert(response == "Hello Cruel World")

server.close()
