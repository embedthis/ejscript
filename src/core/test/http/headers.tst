/*
    Header tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

var http: Http = new Http

http.addHeader("key", "value")
http.get(HTTP + "/index.html")
assert(http.status == 200)

http.get(HTTP + "/index.html")
connection = http.header("connection")
assert(connection == "keep-alive" || connection == "close")
connection = http.header("Connection")
assert(connection == "keep-alive" || connection == "close")

http.get(HTTP + "/index.html?mob=1")
assert(http.contentType == "text/html")
assert(http.date.toString().contains("GMT"))
assert(http.lastModified.toString().contains("GMT"))
http.close()
