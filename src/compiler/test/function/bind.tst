/*
    Test Function.bind, Function.bound
 */

//  Test default binding
function global_fun() {}
assert(global_fun.bound == null)

class Shape {
    static public function sfun() {}
    public function fun() {}
}
shape = new Shape
assert(shape.fun.bound == shape)
assert(Shape.sfun.bound == Shape)

//  Function.bind
let obj = {}
function gg(a, b, c, d) {
    assert(this == obj)
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
    assert(d == undefined)
}
assert(gg.bound == null)
gg.bind(obj, 1, 2)
gg(3)
