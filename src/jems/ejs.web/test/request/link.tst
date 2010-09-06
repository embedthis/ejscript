/*
    Request.link
 */
require ejs.web

r = new Request("http://example.com/dir/list.ejs#ref?a=b")

assert(r.link("c.html") == "/dir/c.html")
assert(r.link("../c.html") == "/c.html")
assert(r.link("/c.html") == "/c.html")

assert(r.link("c.html").local == "/dir/c.html")
assert(r.link("../c.html").local == "/c.html")
assert(r.link("/c.html").local == "/c.html")

assert(r.link("c.html").complete(r.uri) == "http://example.com/dir/c.html")
assert(r.link("../c.html").complete(r.uri) == "http://example.com/c.html")
assert(r.link("/c.html").complete(r.uri) == "http://example.com/c.html")

//  No controller defined and no route - action ignored
assert(r.link({action: "login"}) == "/")

assert(r.link({
    scheme: "https", host: "example.com", port: 8080, path: "/some.html", query: "color=red", reference: "42"}) ==
    "https://example.com:8080/some.html#42?color=red")


r = new Request("http://example.com/Product/update")
assert(r.link("abc") == "/Product/abc")
assert(r.link("") == "/Product")
assert(r.link(".") == "/Product/")

