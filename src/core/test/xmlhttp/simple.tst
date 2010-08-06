/*
    Simple XMLHttp tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
var TIMEOUT = 30000

xh = new XMLHttp
// xh.http.trace(1, ["conn", "first", "headers", "request", "response", "body"])
xh.open("GET", HTTP + "/test.xml")
xh.send(null)

xh.onreadystatechange = function () {
    if (xh.readyState == XMLHttp.Loaded) {
        App.emitter.fire("complete")
    }
}
App.waitForEvent(App.emitter, "complete", xh.http.timeout)

assert(xh.readyState == XMLHttp.Loaded)
assert(xh.status == 200)
assert(xh.statusText == "OK")
assert(xh.responseText.length > 10)
assert(xh.responseText.contains("</order>"))
assert(Object.getType(xh.responseXML) == XML)
assert(xh.responseXML.length() == 2)
assert(xh.getAllResponseHeaders().contains("date"))

// print("State " + xh.readyState)
// print("Status " + xh.status)
// print("StatusText " + xh.statusText)
// print("Response " + xh.responseText)
// print("Headers " + xh.getAllResponseHeaders())
// print("Response " + xh.responseText)
// print("XML " + xh.responseXML)
