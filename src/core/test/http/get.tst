/*
    Get tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.get(HTTP + "/index.html")
assert(http.status == 200)
assert(http.response.contains("Hello /index.html"))
assert(!http.isSecure)
http.close()

http.get(HTTP + "/index.html")
assert(http.readString(6) == "<html>")
assert(http.readString(6) == "<head>")
http.close()
