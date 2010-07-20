/*
    Post tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http
http.post(HTTP + "/index.html", "Some data")
assert(http.status == 200)
http.close()

