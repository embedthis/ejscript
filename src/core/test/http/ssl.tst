/*
    SSL tests
 */

const HTTP = (global.session && session["http"]) || ":6700"
const HTTPS = (global.session && session["https"]) || ":6700"

var http: Http = new Http

if (test.config["ssl"] == 1) {
    http.get(HTTPS)
    assert(http.status == 200)
    assert(http.isSecure)
    http.close()
} else {
    test.skip("SSL not enabled")
}
