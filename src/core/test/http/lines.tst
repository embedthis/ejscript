/*
    Http class tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

var http: Http = new Http

http.get(HTTP + "/index.html")
assert(http.status == 200)
lines = http.readLines()
assert(lines.length == 3)
for (l in lines) {
    assert(lines[l] != "")
}
http.close()
