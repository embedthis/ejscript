/*
    Http class tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http

http.get(HTTP + "/index.html")
assert(http.status == 200)
lines = http.readLines()
assert(lines.length == 3)
for (l in lines) {
    assert(lines[l] != "")
}
http.close()
