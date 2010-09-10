/*
    Router constructors
 */
require ejs.web

//  Empty router
var router = new Router
assert(router)
assert(router.routes)
assert(Object.getOwnPropertyCount(router.routes) == 0)


//  Empty router
var router = new Router
assert(router)
assert(router.routes)
router.add("/something")
assert(Object.getOwnPropertyCount(router.routes) == 1)


//  Default route set
var router = new Router(Router.Default)
assert(router)
assert(router.routes)
assert(Object.getOwnPropertyCount(router.routes) > 1)
for each (name in ["es", "ejs", "dir", "catchall", "static/default"]) {
    assert(router.lookup(name))
}


//  Restful route set
var router = new Router(Router.Restful)
assert(router)
assert(router.routes)
assert(Object.getOwnPropertyCount(router.routes) > 1)
for each (name in ["es", "ejs", "dir", "catchall", "default", "static/default"]) {
    assert(router.lookup(name))
}
for each (name in ["init", "create", "edit", "show", "update", "destroy", "default", "catchall"]) {
    assert(router.lookup(name))
}
