/*
    Test enumerability of class properties
 */

class Shape {
    enumerable var x = 1
    var y = 2
}
s = new Shape
assert(s.x == 1)
assert(s.y == 2)

//  Only x is enumerable
let found = false
for (let [name,value] in s) {
    assert(name == "x")
    assert(value == "1")
}
