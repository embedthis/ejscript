/*
    Closure based inheritance
 */

function Shape(h, w) {
    /* private vars */
    var height = h,
        width = w

    /* Public vars */
    this.marker = "Hello"

    /* private method */
    function calcArea() {
        return width * height
    }

    /* public method */
    this.area = function() {
        return calcArea()
    }
}

//  Create new object

s = new Shape(100, 200)
assert(Object.getOwnPrototypeOf(s) === Shape.prototype)
assert(Object.getOwnPropertyNames(s) == "marker,area")
assert(Shape.prototype.isPrototypeOf(s))
assert(s.marker == "Hello")
assert(s.area() == 20000)

//  Create another object

s2 = new Shape(1, 2)
assert(s2.area() == 2)
assert(s.area() == 20000)
assert(s.prototype === s2.prototype)


//  Subclassing using classical prototype based inheritance

function Cube(d) {
    this.depth = d
}
Cube.prototype = new Shape(1, 2)

c = new Cube(3)
assert(Object.getOwnPrototypeOf(c) === Cube.prototype)
assert(Object.getOwnPropertyNames(c) == "depth")
assert(Cube.prototype.isPrototypeOf(c))
assert(c.marker == "Hello")
assert(c.area() == 2)
assert(c.depth == 3)

