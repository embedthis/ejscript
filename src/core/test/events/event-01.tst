/*
 	Simple use case for Event, Emitter
 */

var gotEvent = undefined

function callback(e: String, a, b, c): Void {
	gotEvent = e
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
}

public var events: Emitter = new Emitter
events.addListener("simple", callback)
events.emit("simple", 1, 2, 3)

assert(gotEvent != undefined)
assert(gotEvent == "simple")
