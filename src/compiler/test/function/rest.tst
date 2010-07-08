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
