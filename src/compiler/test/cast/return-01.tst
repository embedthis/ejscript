/*
    Test casting return types
 */

function nop(arg) {
    return arg
}

n = nop(77)
assert(nop(77) is Number)
assert(nop("abc") is String)
assert(nop({}) is Object)


//  Cast to Number on return

function num(arg): Number {
    return arg
}
n = num(77)
assert(num(77) is Number)
assert(num("abc") is Number)
assert(num({}).isNaN)
