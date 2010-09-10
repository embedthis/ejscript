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
