/*
    Async tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
const TIMEOUT = 10000

var http: Http = new Http
http.async = true

var complete

var buf = new ByteArray
http.on("readable", function (event, http) {
    http.read(buf, -1)
})
http.on("close", function (event, http) {
    complete = true
})

http.get(HTTP + "/index.html")
let mark = new Date
while (!complete && mark.elapsed < TIMEOUT) {
    App.eventLoop(TIMEOUT - mark.elapsed, 1)
}

assert(buf.readString(6) == "<html>")
assert(complete)
assert(http.status == 200)
http.close()
