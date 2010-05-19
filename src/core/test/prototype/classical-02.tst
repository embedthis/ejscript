/*
    Classical prototype inheritance doing "new" before modifying the prototype
 */

function Shape(height, width) {
    this.height = height
    this.width = width
    return this
}

//  Create new objects

s = new Shape(1, 2)
s2 = new Shape(2, 4)

Shape.prototype.marker = "Hello"
Shape.prototype.area = function() {
    return this.height * this.width
}

assert(Object.getOwnPrototypeOf(s) === Shape.prototype)
assert(Object.getOwnPropertyNames(s) == "height,width")
assert(Shape.prototype.isPrototypeOf(s))
assert(s.marker == "Hello")
assert(s.area() == 2)
assert(s2.marker == "Hello")
assert(s2.area() == 8)

//  Modify s and then s2 should be unchanged

s.height = 2
s.marker = "Goodbye"
assert(s.area() == 4)
assert(s.marker == "Goodbye")
assert(s2.area() == 8)
assert(s2.marker == "Hello")

