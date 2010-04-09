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

var gotEvent

function eventCallback(e: String, data): Void {
	assert(e == "keyboard")
	assert(data.key == "PageUp")
    gotEvent = true
}

var s: Shape = new Shape
s.events.addListener("keyboard", eventCallback)
s.events.emit("keyboard", KeyboardEvent("PageUp"))
assert(gotEvent)
