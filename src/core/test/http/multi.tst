/*
    Http class tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

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
