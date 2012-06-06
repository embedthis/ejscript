/*
    Http.fetch tests
 */

let HTTP = App.config.uris.http
let http = new Http
http.fetch("GET", HTTP + "/index.html", null, function() {
    assert(this == http)
    assert(status == 200)
    assert(response.length > 0)
})
App.waitForEvent(http, "close", 10000)
assert(http.status == 200)
assert(http.response.length > 0)
http.close()
