/*
    Nested routes
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
let router = new Router(null)
router.setDefaultBuilder(builder)


//  Nested routes /Dash(/{product}(/{branch}(/{configuration})))

router.reset()
outer = router.add("/Dash", {action: "/Dash/", name: "products", params: {level: 1, dash: "enabled"}})
outer = router.add("/{product}", {action: "/Dash/", name: "branches", params: {level: 2}, outer: outer})
outer = router.add("/{branch}", {action: "/Dash/", name: "configurations", params: {level: 3}, outer: outer})
        router.add("/{configuration}", {action: "/Dash/", name: "default", params: {level: 4}, outer: outer})
let response = test("/Dash/appweb/master/with-ssl")
assert(Object.getOwnPropertyCount(response.params) == 6)
assert(response.params.dash == "enabled")
assert(response.params.product == "appweb")
assert(response.params.branch == "master")
assert(response.params.configuration == "with-ssl")
assert(response.params.level == 4)

server.close()
