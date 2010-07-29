/*
    simple.tst -- Test simple static file retrieval
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.get(HTTP + "/simple.html")
assert(http.status == 200)
assert(http.response == Path("web/simple.html").readString())
http.close()
