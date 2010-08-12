/*
    Test bare Controller object outside a web server
 */

require ejs.web

c = new Controller

assert(c)
assert(c.actionName == null)
assert(c.config == null)
assert(c.controllerName == "-DefaultController-")
assert(c.log == null)
assert(c.params == null)
assert(!c.rendered)
assert(c.request == null)
assert(c.view == null)
assert(c.flashMessages == null)
assert(c.session == null)

assert(c.absHome == null)
assert(c.home == null)
assert(c.uri == null)


//  Now with a request

r = new Request("/index.html")
c = new Controller(r)
assert(c)
assert(c.actionName == null)
assert(c.config)
assert(c.controllerName == "-DefaultController-")
assert(c.log is Logger)
assert(c.params && Object.getOwnPropertyCount(c.params) == 0)
assert(!c.rendered)
assert(c.request == r)
assert(c.view == null)
assert(c.flashMessages == null)
assert(c.session == null)

assert(c.absHome == null)
assert(c.home == null)
assert(c.uri == "/index.html")
