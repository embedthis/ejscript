/*
    Router.remove()
 */
require ejs.web

let request = new Request("http://example.com:7777/app/index.html#section-1?a=b&c=d")
assert(request)

let router = new Router
router.add("/path")
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("path/index")
assert(route.name == "index")
assert(route.template == "/path")

//  Remove
router.remove("path/index")
assert(router.lookup("path/index") == null)
