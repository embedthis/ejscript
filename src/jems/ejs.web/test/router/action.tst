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
    router.route(request)
    write(serialize(request) + "\n")
    finalize()
})

//  Create the router and set the default buidler
let router = new Router
router.setDefaultBuilder(builder)


/*NOT OK
//  /path/next/last. Should map to controller: path, action: next

router.reset()
router.add("/User/{action}")
let response = test("/User/register")
assert(response.pathInfo == "/User/register")
dump(response)
assert(Object.getOwnPropertyCount(response.params) == 2)
assert(response.params.controller == "path")
assert(response.params.action == "next")
*/

server.close()
