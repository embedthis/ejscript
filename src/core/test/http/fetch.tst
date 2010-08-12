/*
    Http.fetch tests
 */

ENDPOINT = "http://embedthis.com"

let HTTP = ":" + (App.config.test.http_port || "6700")
let http = new Http
http.fetch("GET", ENDPOINT + "/index.html", null, function() {
    assert(this == http)
    assert(status == 200)
    assert(response.length > 0)
})
App.waitForEvent(http, "close", 10000)
assert(http.status == 200)
assert(http.response.length > 0)
