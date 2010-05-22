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
assert(!(delete s.shape))

//  ECMA says delete should return true for non-existant properties
assert((delete s.missing))


//  check property attributes

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

assert(!(delete e.temporary))


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
