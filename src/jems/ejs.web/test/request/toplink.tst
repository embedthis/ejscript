/*
    Request.toplink
 */
require ejs.web

r = new Request("http://example.com/dir/list.ejs#ref?a=b")
r.absHome = Uri("/")
r.home = Uri("../../")

assert(r.toplink("c.html") == "/c.html")
assert(r.toplink("../c.html") == "/c.html")
assert(r.toplink("/c.html") == "/c.html")


assert(r.toplink("c.html").complete(r.uri) == "http://example.com/c.html")
assert(r.toplink("../c.html").complete(r.uri) == "http://example.com/c.html")
assert(r.toplink("/c.html").complete(r.uri) == "http://example.com/c.html")

//  No controller defined and no route
assert(r.toplink({action: "login"}) == "/login")

assert(r.toplink({
    scheme: "https", host: "example.com", port: 8080, path: "/some.html", query: "color=red", reference: "42"}) ==
    "https://example.com:8080/some.html#42?color=red")

