/*
    Router.add*()
 */
require ejs.web

//  add

let router = new Router
router.add("/path")
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("path/index")
assert(route)
assert(route.name == "index")
assert(route.template == "/path")
assert(Object.getOwnPropertyCount(route.params) == 0)


//  addDefault

let router = new Router
router.addDefault()
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("default")
assert(route)
assert(route.name == "default")
assert(route.template == "/{controller}/{action}")
assert(route.tokens == "controller,action")


//  addHome

let router = new Router
router.addHome("/home.html")
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("home")
assert(route)
assert(route.name == "home")
assert(route.template == "/")
assert(route.tokens == null)
assert(Object.getOwnPropertyCount(route.params) == 0)


//  addCatchall

let router = new Router
router.addCatchall()
assert(Object.getOwnPropertyCount(router.routes) == 1)
var route = router.lookup("catchall")
assert(route)
assert(route.name == "catchall")
assert(route.template == "/^\\/.*$/")
assert(route.tokens == null)


//  addHandlers

let router = new Router
router.addHandlers()
assert(Object.getOwnPropertyCount(router.routes) == 2)
for each (name in ["es", "ejs", "dir", "static/default"]) {
    assert(router.lookup(name))
}


//  addResource

let router = new Router
router.addResource("test")
assert(Object.getOwnPropertyCount(router.routes) == 1)
assert(route = router.lookup("test/edit"))
assert(route.name == "edit")
assert(route.template == "/test/edit")
assert(route.method == "GET")

assert(route = router.lookup("test/show"))
assert(route.name == "show")
assert(route.template == "/test")
assert(route.method == "GET")

assert(route = router.lookup("test/update"))
assert(route.name == "update")
assert(route.template == "/test")
assert(route.method == "PUT")

assert(route = router.lookup("test/default"))
assert(route.name == "default")
assert(route.template == "/test/{action}")
assert(route.method == "")

//  addResource([])
let router = new Router
router.addResource(["one", "two", "three"])
assert(Object.getOwnPropertyCount(router.routes) == 3)
assert(route = router.lookup("one/default"))
assert(route = router.lookup("two/default"))
assert(route = router.lookup("three/default"))


//  addResources

let router = new Router
router.addResources("test")
assert(Object.getOwnPropertyCount(router.routes) == 1)
assert(route = router.lookup("test/init"))
assert(route.name == "init")
assert(route.template == "/test/init")
assert(route.method == "GET")

assert(route = router.lookup("test/index"))
assert(route.name == "index")
assert(route.template == "/test")
assert(route.method == "GET")

assert(route = router.lookup("test/create"))
assert(route.name == "create")
assert(route.template == "/test")
assert(route.method == "POST")

assert(route = router.lookup("test/edit"))
assert(route.name == "edit")
assert(route.template == "/test/{id}/edit")
assert(route.method == "GET")

assert(route = router.lookup("test/show"))
assert(route.name == "show")
assert(route.template == "/test/{id}")
assert(route.method == "GET")

assert(route = router.lookup("test/update"))
assert(route.name == "update")
assert(route.template == "/test/{id}")
assert(route.method == "PUT")

assert(route = router.lookup("test/destroy"))
assert(route.name == "destroy")
assert(route.template == "/test/{id}")
assert(route.method == "DELETE")

assert(route = router.lookup("test/default"))
assert(route.name == "default")
assert(route.template == "/test/{action}")
assert(route.method == "")


//  addResources([])
let router = new Router
router.addResources(["one", "two", "three"])
assert(Object.getOwnPropertyCount(router.routes) == 3)
assert(route = router.lookup("one/default"))
assert(route = router.lookup("two/default"))
assert(route = router.lookup("three/default"))


//  addRestful
let router = new Router
router.addRestful()
assert(Object.getOwnPropertyCount(router.routes) == 1)

assert(route = router.lookup("init"))
assert(route.name == "init")
assert(route.template == "/{controller}/init")
assert(route.method == "GET")

assert(route = router.lookup("index"))
assert(route.name == "index")
assert(route.template == "/{controller}")
assert(route.method == "GET")

assert(route = router.lookup("create"))
assert(route.name == "create")
assert(route.template == "/{controller}")
assert(route.method == "POST")

assert(route = router.lookup("edit"))
assert(route.name == "edit")
assert(route.template == "/{controller}/{id}/edit")
assert(route.method == "GET")

assert(route = router.lookup("show"))
assert(route.name == "show")
assert(route.template == "/{controller}/{id}")
assert(route.method == "GET")

assert(route = router.lookup("update"))
assert(route.name == "update")
assert(route.template == "/{controller}/{id}")
assert(route.method == "PUT")

assert(route = router.lookup("destroy"))
assert(route.name == "destroy")
assert(route.template == "/{controller}/{id}")
assert(route.method == "DELETE")

assert(route = router.lookup("default"))
assert(route.name == "default")
assert(route.template == "/{controller}/{action}")
assert(route.method == "")
