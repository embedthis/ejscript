/*
    Test the async.es - This is an async script. Use an async client as well for good measure.
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
const TIMEOUT = 10000

var http: Http = new Http
http.async = true

var complete

var buf = new ByteArray
http.observe("readable", function (event, http) {
    http.read(buf, -1)
})
http.observe("close", function (event, http) {
    complete = true
})

http.get(HTTP + "/async.es")
let mark = new Date
while (!complete && mark.elapsed < TIMEOUT) {
    App.eventLoop(TIMEOUT - mark.elapsed, 1)
}

assert(http.status == 200)
assert(buf == "Now done\n")
assert(complete)
http.close()
