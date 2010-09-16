/*
    Router Action / Controller definition
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


//  add("/User/{action}") -- Set action/controller from the template

router.reset()
router.add("/User/{action}")
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 1)
assert(response.params.action == "register")


//  Override controller: add("/User/{action}", {controller: "Staff"})

router.reset()
router.add("/User/{action}", {controller: "Staff"})
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "Staff")
assert(response.params.action == "register")


//  Default action too: add("/User/{action}", {controller: "Staff", action: "authorize"})

router.reset()
router.add("/User/register", {controller: "Staff", action: "authorize"})
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "Staff")
assert(response.params.action == "authorize")


//  Default action with "controller/action"

router.reset()
router.add("/User/register", {action: "Staff/discount"})
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "Staff")
assert(response.params.action == "discount")


//  Set with @literal - controller only

router.reset()
router.add("/User/{action}", "@Staff/")
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "Staff")
assert(response.params.action == "register")


//  Set with @literal

router.reset()
router.add("/{controller}/{action}", "@discount")
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "User")
assert(response.params.action == "discount")

server.close()
