/*
    Http class tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http

http.chunked = true
http.post(HTTP + "/index.html")
assert(http.status == 200)

http = new Http
http.chunked = false
http.post(HTTP + "/index.html")
assert(http.status == 200)

http.close()
