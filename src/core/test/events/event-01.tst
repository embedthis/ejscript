/*
 	Simple use case for Event, Emitter
 */

var fired = undefined

function callback(e: String, a, b, c): Void {
	fired = e
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
}

public var events: Emitter = new Emitter
events.observe("simple", callback)
events.fire("simple", 1, 2, 3)

assert(fired != undefined)
assert(fired == "simple")
