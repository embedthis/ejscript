/*
    Head tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http

http.head(HTTP + "/index.html")
assert(http.status == 200)
assert(http.response == "")
assert(http.contentLength > 0) 

http.close()
