/*
    SSL tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)
const HTTPS = "https://127.0.0.1:" + ((global.test && test.config.ssl_port) || 6743)

var http: Http = new Http

if (test.config.ssl) {
    http.get(HTTPS + "/index.html")
    assert(http.status == 200)
    assert(http.isSecure)
    http.close()

} else {
    test.skip("SSL not enabled")
}
