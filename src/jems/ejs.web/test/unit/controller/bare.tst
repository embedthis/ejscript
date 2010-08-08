/*
    Test bare Controller object outside a web server
 */

require ejs.web

c = new Controller

assert(c)
assert(c.actionName == null)
assert(c.config == null)
assert(c.controllerName == null)
assert(c.deploymentMode == null)
assert(c.log == null)
assert(c.params == null)
assert(c.request == null)
assert(c.view == null)
assert(c.flash == null)
assert(c.session == null)


//  Now with a request

r = new Request
c = new Controller(r)
assert(c)
assert(c.actionName == null)
assert(c.config)
assert(c.controllerName == "-Controller-")
assert(c.deploymentMode == null)
assert(c.log is Logger)
assert(c.params && Object.getOwnPropertyCount(c.params) == 0)
assert(c.request == r)
assert(c.view == null)
assert(c.flash == null)
assert(c.session == null)
