/*
    Object.create creation using classical inheritance
 */

function Shape(height, width) {
    this.height = height
    this.width = width
    return this
}
Shape.prototype.marker = "Hello"
Shape.prototype.area = function() {
    return this.height * this.width
}

s = Object.create(Shape.prototype, {height: {value: 5}, width: {value: 2}})
assert(Object.getOwnPrototypeOf(s) === Shape.prototype)
assert(Object.getOwnPropertyNames(s) == "height,width")
assert(Shape.prototype.isPrototypeOf(s))
assert(s.marker == "Hello")
assert(s.area() == 10)
