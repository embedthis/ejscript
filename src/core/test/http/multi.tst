/*
    Http class tests
 */

const HTTP = App.config.uris.http
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
