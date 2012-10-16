/*
    Header tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.setHeader("key", "value")
http.get(HTTP + "/index.html")
assert(http.status == 200)

http.get(HTTP + "/index.html")
connection = http.header("connection")
assert(connection == "Keep-Alive" || connection == "close")
connection = http.header("Connection")
assert(connection == "Keep-Alive" || connection == "close")

http.get(HTTP + "/index.html?mob=1")
assert(http.contentType == "text/html")
assert(http.header("Date").contains("GMT"))
http.close()
