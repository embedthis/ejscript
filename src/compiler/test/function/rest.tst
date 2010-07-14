/*
  	Test rest args
 */

function testRest(a, b, c, ...items)
{
	assert(a is Number)
	assert(b is String)
	assert(c is boolean)
	assert(items == "4,5,6")
	assert(items is Array)
	assert(items.length == 3)
}
testRest(1, "Hello World", true, 4, 5, 6)


//  Test rest with prior default args
function defaultRest(a = 7, ...items) {
    assert(a is Number)
    assert(items is Array)
    return [a] + items
}
assert(defaultRest() == "7")
assert(defaultRest(1) == "1")
assert(defaultRest(1, 2, 3) == "1,2,3")
