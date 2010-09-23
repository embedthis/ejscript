/*
    Test function calls with namespace qualifiers
 */
namespace action

var called = false

action function f1() {
    called = true
}

function f2() {
    action::f1()
}
f2()
assert(called)



//  Inside a class
var state

class Shape {
    action function f1(arg) {
        state = arg
    }
    function f2() {
        action::f1(1)
        assert(state == 1)

        this.action::f1(2)
        assert(state == 2)

        this."action"::["f1"](3)
        assert(state == 3)

        use namespace action
        f1(4)
        assert(state == 4)
    }
}

s = new Shape
s.f2()
assert(state == 4)
