/*
  	Function.name
 */

function fun() {
    assert(fun.name == "fun")

    function inner() {}
    assert(inner.name == "inner")

    var lambda = function () true
    assert(lambda.name == "")
}
fun()
