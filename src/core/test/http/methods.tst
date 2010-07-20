/*
    Method tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)
var http: Http = new Http

test.skip("method")

http.options(HTTP + "/index.html")
assert(http.header("Allow") == "OPTIONS,GET,HEAD,POST,PUT,DELETE")
http.close()

http.trace(HTTP + "/index.html")
assert(http.status == 406)
http.close()
