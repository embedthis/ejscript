/*
    SSL tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
const HTTPS = "https://127.0.0.1:" + (App.config.test.ssl_port || "6743")

var http: Http = new Http

if (App.config.test.ssl) {
    http.get(HTTPS + "/index.html")
    assert(http.status == 200)
    assert(http.isSecure)
    http.close()

} else {
    if (global.test) {
        test.skip("SSL not enabled")
    }
}
