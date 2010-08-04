/*
    Test bare Request object outside a web server
 */

require ejs.web

r = new Request
assert(r)
try {
    assert(r.uri == null)
} catch (e) {
    assert(e is IOError)
}
