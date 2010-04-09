/*
    Http class tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

var http: Http = new Http

http.chunked = true
http.post(HTTP + "/index.html")
assert(http.status == 200)

http = new Http
http.chunked = false
http.post(HTTP + "/index.html")
assert(http.status == 200)

http.close()
