/*
    Request.makeUriHash
 */
require ejs.web

r = new Request("http://example.com/dir/list.ejs#ref?a=b")

assert(r.makeUriHash("c.html").uri == "c.html")
assert(r.makeUriHash("../c.html").uri == "../c.html")
assert(r.makeUriHash("/c.html").uri == "/c.html")
