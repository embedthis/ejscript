/*
  	Spread args
 */

function testArraySpread(a, b, c) {
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
}

testArraySpread(... [1,2,3])
a = [1,2,3]
testArraySpread(...a)


function testObjectSpread(a, b, c) {
    assert(a == 1)
    assert(b == 2)
    assert(c == 3)
}
o = {a: 1, b: 2, c: 3}
testObjectSpread(...o)
testObjectSpread(...{a: 1, b: 2, c: 3})
