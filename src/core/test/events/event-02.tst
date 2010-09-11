/*
 	Event, Dispatcher in a class with custom event
 */

class KeyboardEvent {

    public var key
	function KeyboardEvent(key: String) {
		this.key = key
	}
}

class Shape {
	public var events: Emitter = new Emitter
}

var fired

function eventCallback(e: String, data): Void {
	assert(e == "keyboard")
	assert(data.key == "PageUp")
    fired = true
}

var s: Shape = new Shape
s.events.on("keyboard", eventCallback)
s.events.fire("keyboard", KeyboardEvent("PageUp"))
assert(fired)
