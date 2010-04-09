/*
    Test ES5 Object and property attributes
 */

var o = {
    shape: "rectangle",
}

d = Object.getOwnPropertyDescriptor(o, "shape")
assert(d.value == "rectangle")
assert(d.configurable)
assert(d.enumerable)
assert(d.writable)
assert(d.type == null)
assert(Object.isExtensible(o))
assert(!Object.isFrozen(o))
assert(!Object.isSealed(o))


//  Class (non-dynamic)

class Shape {
    // use default namespace ""
    public const shape: String = "circle"
    public var temp: Number = 102
}

var s = new Shape

//  must not be extensible via addition or deletion

// assert(!Object.isExtensible(s))
let caught = false
try { delete s.shape; } catch { caught = true; }
assert(caught == true)
caught = false
try { s.width = 10; } catch { caught = true; }
assert(caught == true)


//  check property attributes

assert(caught)
d = Object.getOwnPropertyDescriptor(s, "shape")
assert(!d.configurable)
assert(!d.enumerable)
assert(!d.writable)
assert(d.type == String)
d = Object.getOwnPropertyDescriptor(s, "temp")
assert(!d.configurable)
assert(!d.enumerable)
assert(d.writable)
assert(d.type == Number)


//  Class (non-dynamic)

dynamic class Ellipse {
    // use default namespace ""
    public const shape = "circle"
    public var shade = "blue"
    public var temporary
}

var e = new Ellipse

assert(e.shape == "circle")
assert(e.shade == "blue")
assert(Object.isExtensible(e))

//  Add new property

e.width = 10

assert(e.width == 10)
let caught = false

//  Deleting class fixtures should still fail

try { delete e.temporary; } catch { caught = true; }
assert(caught)

//  Test attributes 
d = Object.getOwnPropertyDescriptor(e, "shape")
assert(!d.configurable)
assert(!d.enumerable)
assert(!d.writable)

d = Object.getOwnPropertyDescriptor(e, "shade")
assert(!d.configurable)
assert(!d.enumerable)
assert(d.writable)

d = Object.getOwnPropertyDescriptor(e, "width")
assert(d.configurable)
assert(d.namespace == "")
assert(d.enumerable)
assert(d.writable)

