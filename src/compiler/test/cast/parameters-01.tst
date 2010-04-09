/*
    Test casting parameter types
 */

//  The first three will cast values

function fun(a: Number, b: String, c: Boolean, d: String?)
{
    assert(a is Number)
    assert(b is String)
    assert(c is Boolean)
    assert(d is String || d == undefined || d == null)
}

fun(1, "77", true, "hello")
fun("1", 77, false, null)
fun("1", 77, false, undefined)
fun("1234", "1234", "1234", undefined)
