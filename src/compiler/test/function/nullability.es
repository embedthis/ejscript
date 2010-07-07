/*
    Test nullability type operators in parameters
 */

function sfun(a, b: String, c: String!): String {
    return typeOf(a) + " " + typeOf(b) + " " + typeOf(c)
}

assert(sfun("a", "b", "c")                      == "String String String")
assert(sfun(null, "str", "c")                   == "Null String String")
assert(sfun(1, 2, "c")                          == "Number String String")
assert(sfun("a", "b", "c", "d", "e", "f")       == "String String String")


//  Fail cases
caught = false ; try { sfun(); } catch { caught = true; } ; assert(caught)
caught = false ; try { sfun(null, null, null); } catch { caught = true; } ; assert(caught)
caught = false ; try { sfun("abc", "str", null); } catch { caught = true; } ; assert(caught)
caught = false ; try { sfun("abc", "str", undefined); } catch { caught = true; } ; assert(caught)


//  Same but with Numbers

function nfun(a, b: Number, c: Number!): String {
    return typeOf(a) + " " + typeOf(b) + " " + typeOf(c)
}

assert(nfun(1, 2, 3)                            == "Number Number Number")
assert(nfun("1", "2", 3)                        == "String Number Number")
assert(nfun(1, 2, 3, 4, 5, 6, 7)                == "Number Number Number")

//  Fail cases
caught = false ; try { nfun(); } catch { caught = true; } ; assert(caught)
caught = false ; try { nfun(null, null, null); } catch { caught = true; } ; assert(caught)


//  Casting

function cfun(msg: String~): Object {
    assert(msg is String)
    return msg
}

assert(cfun("hello") is String)
assert(cfun(77) is String)
assert(cfun(null) is String)
assert(cfun(null) == "null")
