/*
    Test ES5 Object manipulation routines
 */

var master = {
    shape: "rectangle",
    color: "blue",
    height: 175,
}
var o = master.clone()


//  General object interrogation

assert(o.hasOwnProperty("shape") == true)
assert(o.hasOwnProperty("height") == true)
assert(o.hasOwnProperty("xxx") == false)
assert(o.hasOwnProperty("clone") == false)
assert(o.hasOwnProperty("length") == false)
assert(o.propertyIsEnumerable("shape"))
assert(Object.isExtensible(o))
assert(!Object.isFrozen(o))
assert(!Object.isSealed(o))
assert(Object.keys(o) == "shape,color,height")
assert(Object.getOwnPropertyNames(o) == "shape,color,height")

d = Object.getOwnPropertyDescriptor(o, "shape")
assert(d.value == "rectangle")
assert(d.configurable)
assert(d.enumerable)
assert(d.writable)
assert(d.type == null)
assert(d.namespace == "")


//  defineProperty

Object.defineProperty(o, "width", {
    configurable: true,
    enumerable: true,
    writable: true,
    namespace: "",
    get: function() { return this._width; },
    set: function(v) { this._width = v; },
})


//  defineProperties (redefining in this case)

Object.defineProperties(o, {"width": { value: 10 }, "height": { value: 20, type: Number }})
assert(o.width == 10)
assert(o.height == 20)
o.width = o.height = 30
assert(o.width == 30)
assert(o.height == 30)
assert(Object.getOwnPropertyDescriptor(o, "height").width == null)
assert(Object.getOwnPropertyDescriptor(o, "height").type == Number)


//  PreventExtensions

assert(Object.isExtensible(o))
Object.preventExtensions(o)
assert(!Object.isExtensible(o))
assert(!Object.isFrozen(o))
assert(!Object.isSealed(o))


//  Seal

Object.seal(o)
assert(Object.isSealed(o))
assert(!Object.isFrozen(o))


//  Freeze

Object.freeze(o)
assert(Object.isFrozen(o))
assert(Object.isSealed(o))


//  getOwnPrototypeOf

function MyShape(height, width) {
    this.height = height
    this.width = width
    return this
}
s = new MyShape(100, 200)
// assert(Object.getOwnPrototypeOf(s) === Reflect(s).type)

assert(Object.getOwnPrototypeOf(s) === MyShape.prototype)
assert(MyShape.prototype.isPrototypeOf(s))
proto = MyShape.prototype
proto.marker = "Hello"
proto.area = function() {
    return this.height * this.width
}
assert(s.marker == "Hello")
assert(s.area() == 20000)


//  Create another shape to ensure prototype is working

s2 = new MyShape(1, 2)
assert(s.area() == 20000)
assert(s2.area() == 2)


//  create

s3 = Object.create(MyShape.prototype, {height: {value: 5}, width: {value: 2}})
assert(s3.area() == 10)
