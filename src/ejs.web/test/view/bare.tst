/*
    Test bare View object outside a web server
 */

require ejs.web

v = new View
assert(v)

assert(v.config == App.config)
assert(v.controller == null)
assert(v.currentRecord == null)
assert(v.request == null)

//  Now with a request

r = new Request("/index.html")
v = new View(r)
assert(v)

assert(v.config == r.config)
assert(v.controller == null)
assert(v.currentRecord == null)
assert(v.request == r)
