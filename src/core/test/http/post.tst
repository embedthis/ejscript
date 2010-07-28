/*
    Post tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.post(HTTP + "/index.html", "Some data")
assert(http.status == 200)
http.close()

