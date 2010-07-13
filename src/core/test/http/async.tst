/*
    Async tests
 */

const HTTP = (global.session && session["http"]) || ":6700"
const TIMEOUT = (global.session && session["timeout"]) || 30000
var http: Http = new Http
http.async = true
var buf = new ByteArray
var complete

http.observe("readable", function (event, http) {
    assert(http.status == 200)
    http.read(buf)
})
http.observe("complete", function (event, http) {
    complete = true
})

http.get(HTTP + "/index.html")
http.finalize()

let mark = new Date
while (!complete && mark.elapsed < TIMEOUT) {
    App.eventLoop(TIMEOUT - mark.elapsed, 1)
}

assert(buf.readString(6) == "<html>")
assert(complete)

http.close()
