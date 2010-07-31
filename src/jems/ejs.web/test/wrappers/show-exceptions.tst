/*
    Test ShowExceptions wrapper
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/dispatch.es?route=show")
assert(http.response.contains("Expected Big Bang"))
assert(http.status == Http.ServerError)
assert(http.contentLength > 0)
http.close()

