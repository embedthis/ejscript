/*
    Simple WebSocket tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
var TIMEOUT = 30000

let ws = new WebSocket("ws://localhost" + HTTP")
ws.send("Hello WebSocket World")

ws.onopen = function (event) {
    print("THIS", this)
    ws.send("Thanks for opening")
    //  console.log(event.data)
}

print("TIMEOUT is", ws.http.timeout)
ws.close()

ws.onreadystatechange = function () {
    if (ws.readyState == WebSocket.Loaded) {
        App.emitter.fire("complete")
    }
}
App.waitForEvent(App.emitter, "complete", ws.http.timeout)

assert(ws.readyState == WebSocket.Loaded)
assert(ws.status == 200)
assert(ws.statusText == "OK")
assert(ws.responseText.length > 10)
assert(ws.responseText.contains("</order>"))
assert(Object.getType(ws.responseXML) == XML)
assert(ws.responseXML.length() == 2)
assert(ws.getAllResponseHeaders().contains("date"))

// print("State " + ws.readyState)
// print("Status " + ws.status)
// print("StatusText " + ws.statusText)
// print("Response " + ws.responseText)
// print("Headers " + ws.getAllResponseHeaders())
// print("Response " + ws.responseText)
// print("XML " + ws.responseXML)
