/*
    Get tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.setHeader("Accept-Encoding", "gzip")
http.get(HTTP + "/compressed.txt")
assert(http.status == 200)
http.close()
