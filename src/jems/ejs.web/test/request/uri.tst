/*
    Request.makeUri
 */
require ejs.web

r = new Request("/dir/list.ejs#ref?a=b")

assert(r.makeUri("c.html") == "/dir/c.html")
assert(r.makeUri("../c.html") == "/c.html")
assert(r.makeUri("/c.html") == "/c.html")

assert(r.makeUri({
    scheme: "https", host: "example.com", port: 8080, path: "/some.html", query: "color=red", reference: "42"}) ==
    "https://example.com:8080/some.html#42?color=red")
