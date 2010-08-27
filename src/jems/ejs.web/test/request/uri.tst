/*
    Request.link
 */
require ejs.web

r = new Request("http://example.com/dir/list.ejs#ref?a=b")

assert(r.link("c.html") == "http://example.com/dir/c.html")
assert(r.link("../c.html") == "http://example.com/c.html")
assert(r.link("/c.html") == "http://example.com/c.html")

assert(r.link("c.html").local == "/dir/c.html")
assert(r.link("../c.html").local == "/c.html")
assert(r.link("/c.html").local == "/c.html")

assert(r.link("c.html").relative(r.uri) == "c.html")
assert(r.link("../c.html").relative(r.uri) == "../c.html")
assert(r.link("/c.html").relative(r.uri) == "../c.html")

assert(r.link({
    scheme: "https", host: "example.com", port: 8080, path: "/some.html", query: "color=red", reference: "42"}) ==
    "https://example.com:8080/some.html#42?color=red")

