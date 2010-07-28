/*
    Get tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/index.html")
assert(http.status == 200)
assert(http.response.contains("Hello /index.html"))
assert(!http.isSecure)

http.get(HTTP + "/index.html")
assert(http.readString(6) == "<html>")
assert(http.readString(6) == "<head>")
http.close()

