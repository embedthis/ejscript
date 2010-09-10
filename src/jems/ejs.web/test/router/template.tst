/*
    Router Templates
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
load("helper.es")
server = new HttpServer
server.listen(HTTP)

//  Server - route request and serialize the request object as a response
server.on("readable", function (event, request: Request) {
    router.route(request)
//dump(request)
    write(serialize(request) + "\n")
    finalize()
})


//  Create the router and set the default buidler
let router = new Router
router.setDefaultBuilder(builder)


//  /path/next/last. Should map to controller: path, action: next

router.reset()
router.add("/path/next/last")
let response = test("/path/next/last")
assert(response.pathInfo == "/path/next/last")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "path")
assert(response.params.action == "next")


//  With literal target

router.reset()
router.add("/path/next/last", "/another/path/to/run.html")
assert(route = router.lookup("path/next"))
let response = test("/path/next/last")
assert(response.route.target == "/another/path/to/run.html")


//  using a target

router.reset()
router.add("/path/next/last", { target: "/another/path/to/run.html"})
assert(route = router.lookup("path/next"))
let response = test("/path/next/last")
assert(response.route.target == "/another/path/to/run.html")
assert(Object.getOwnPropertyCount(response.params) == 0)


//  With tokens

router.reset()
router.add("/{color}/{flavor}/{temp}", {name: "test"})
let response = test("/red/mild/100")
assert(Object.getOwnPropertyCount(response.params) == 3)
assert(response.params.color == "red")
assert(response.params.flavor == "mild")
assert(response.params.temp == "100")


//  With optional token

router.reset()
router.add("/{color}(/{flavor})", {name: "test"})
let response = test("/red")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.color == "red")
assert(response.params.flavor == "")


//  Without optional token

let response = test("/red/mild")
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.color == "red")
assert(response.params.flavor == "mild")


//  Param token replacement using {}

router.reset()
router.add("/{color}/{flavor}/{temp}", {params: { summary: "Weather is {temp}, mood is {color}, food is {flavor}"}})
let response = test("/red/mild/100")
assert(response.params.summary == "Weather is 100, mood is red, food is mild")
assert(response.params.color == "red")
assert(response.params.flavor == "mild")
assert(response.params.temp == "100")


//  Param token replacement using $

router.reset()
router.add("/{color}/{flavor}/{temp}", {params: { summary: "$1 $2 $3"}})
let response = test("/red/mild/100")
assert(response.params.summary == "red mild 100")
assert(response.params.color == "red")
assert(response.params.flavor == "mild")
assert(response.params.temp == "100")

server.close()
