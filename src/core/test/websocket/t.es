
let ws = new WebSocket('ws://localhost:4100/websock/test', ['amazing', 'chat'])

assert(ws.readyState == WebSocket.CONNECTING)

ws.onopen = function (event) {
    print("t.es: open")
    ws.send("Dear Server: Thanks for listening")
    print("THIS", this)
    assert(ws.readyState == WebSocket.OPEN)

    ws.close()
    assert(ws.readyState == WebSocket.CLOSING)
}

ws.onmessage = function (event) {
    /* if (event.data instanceof Blob) {
    } else {
    } */
    print("INSTANCEOF", e.data instanceof ByteArray)
    print("TYPEOF", typeof e.data === "string")
    print("t.es: got message: ", event.data)
    assert(ws.readyState == WebSocket.OPEN)
}

ws.onclose = function (event) {
    print("t.es: closed")
    assert(ws.readyState == WebSocket.CLOSED)
}

ws.onerror = function (event) {
    print("t.es: error " + event)
}
// ws.close()

/*
    Test
        ws.binaryType == 'ByteArray'
        readyStates
        send with multiple args
        send with binary
        send with conversion to text
 */
/*
    ws.binaryType = 'arraybuffer'
    event.data.byteLength
    ws.binaryType = 'blob'
    print(ws.extensions)
 */

App.run()

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
