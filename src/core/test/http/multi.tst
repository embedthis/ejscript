/*
    Http class tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.get(HTTP + "/index.html")
assert(http.status == 200)
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.close()
