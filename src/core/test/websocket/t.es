
let ws = new WebSocket('ws://localhost:4100/websock/proto/msg', ['chat'])

ws.onopen = function (event) {
    ws.send("Thanks for opening")
    print(event.data)
}

ws.onmessage = function (event) {
    /* if (event.data instanceof Blob) {
    } else {
    } */
    print("MSG", event.data)
}

ws.onclose = function (event) {
    print("CLOSED")
}

ws.onerror = function (event) {
    print("Error " + event)
}
// ws.close()

/*
    ws.binaryType = 'arraybuffer'
    event.data.byteLength
    ws.binaryType = 'blob'
    print(ws.extensions)
 */

App.run()
// App.waitForEvent(App.emitter, "complete", ws.http.timeout)

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
