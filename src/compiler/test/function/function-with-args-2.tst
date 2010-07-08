/*
  	Function arguments
 */

function fun(a, b)
{
	return "" + a + b
}

assert(fun(1,2) == "12")


function funTyped(a: Number, b: Number)
{
	return "" + a + b
}
assert(funTyped(1,0) == "10")
assert(funTyped(1,2) == "12")

assert(funTyped(1,2,3,4) == "12")
