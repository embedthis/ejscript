/*
    onmessage Tests
 */
var w: Worker

w = new Worker("onmessage.es")
w.onmessage = function (e) {
    assert(e.data.contains("Short Message"))
}
Worker.join()
