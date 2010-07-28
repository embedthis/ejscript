/*
    Reset tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http

http = new Http
http.setHeader("Content-Length", 1000)
http.post(HTTP + "/index.html")
http.reset()
http.setHeader("Content-Length", 1000)
http.post(HTTP + "/index.html")
http.reset()
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.close()
