/*
    Post tests
 */

const HTTP = (global.session && session["http"]) || ":6700"
var http: Http = new Http

http.post(HTTP + "/index.html", "Some data")
assert(http.status == 200)
http.close()

