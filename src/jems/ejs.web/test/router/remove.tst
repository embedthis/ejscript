/*
    Router.remove()
 */
require ejs.web

let request = new Request("http://example.com:7777/app/index.html#section-1?a=b&c=d")
assert(request)

let router = new Router
router.add("/path")
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("path/default")
assert(route.name == "default")
assert(route.template == "/path")

//  Remove
router.remove("path/default")
assert(router.lookup("path/default") == null)
