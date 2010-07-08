/*
    Test setter without a getter
 */

class Shape {
    public var shade = "blank"
    public function set color(value) {
        shade = value
    }
}

var s: Shape = new Shape

assert(s.shade == "blank")
s.color = "blue"
assert(s.shade == "blue")
