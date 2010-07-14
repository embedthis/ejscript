/*
    Simple XMLHttp tests
 */

var ENDPOINT = (global.session && session["http"]) || ":6700"
var TIMEOUT = (global.session && session["timeout"]) || 30000

xh = new XMLHttp
xh.open("GET", ENDPOINT + "/test.xml")
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
