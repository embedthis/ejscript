/*
    Router responses
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
load("helper.es")
server = new HttpServer
server.listen(HTTP)

//  Server - route request and serialize the request object as a response
server.on("readable", function (event, request: Request) {
    try {
        let app = router.route(request)
        Web.process(app, request)
    } catch (e) {
        print(e)
        finalize()
        assert(!e)
    }
    // write(serialize(request) + "\n")
    finalize()
})

//  Create the router and set the default buidler
let router = new Router(null)
router.setDefaultBuilder(builder)


//  Run with inline response

router.reset()
router.add("/custom/test", { run: { status: 200, body: "hello world\n" }})
let response = test("/custom/test")
assert(response == "hello world")


//  Run with function response

router.reset()
router.add("/custom/test", { run: function builder(request) {
    return function app(request) {
        return { body: "Hello Cruel World\n" }
    }
}})
let response = test("/custom/test")
assert(response == "Hello Cruel World")

server.close()
