/*
    Test nullability type operators in variable declarations
 */

/*
    What about redefinitions?
 */

// var x: String?

/*OK
// print(typeOf(x))
breakpoint()
x = "abc"
x = null ; print(typeOf(x))
x = undefined ; print(typeOf(x))
x = "hello" ; print(typeOf(x))
// x = 77 ; print(typeOf(x))
// x = Object ; print(typeOf(x))
// x = { weather: "sunny", temp: 78 }  ; print(typeOf(x))
*/


class Shape {
    public var x: String! = "Hello"
}

s = new Shape

print(s.x)

s.x = "abc"
s.x = null ; print(typeOf(s.x))
s.x = undefined ; print(typeOf(s.x))
s.x = "hello" ; print(typeOf(s.x))
s.x = 77 ; print(typeOf(s.x))
// s.x = Object ; print(typeOf(s.x))
// s.x = { weather: "sunny", temp: 78 }  ; print(typeOf(s.x))


/*
var a
var b: String
var c: String?
var d: String!

a = "a"
b = "b"
c = "c"
d = "d"
*/


/*
assert(sfun("a", "b", "c", "d")                                  == "String String String String")
assert(sfun(null, null, null, "must-be-string")                  == "Null String Null String")
assert(sfun(undefined, undefined, undefined, "must-be-string")   == "Void String Void String")
assert(sfun("a", "b", "c", "d", "e", "f")                        == "String String String String")
assert(sfun(1, 2, "c", "d")                                      == "Number String String String")
assert(sfun(undefined, undefined, undefined, "must-be-string")   == "Void String Void String")
assert(sfun("a", "b", "c", "d", "e", "f")                        == "String String String String")


//  Fail cases
caught = true ; try { sfun(); } catch { caught = true; } ; assert(caught)
caught = true ; try { sfun(null, null, null, null); } catch { caught = true; } ; assert(caught)
caught = true ; try { sfun("a", "b", "c", null); } catch { caught = true; } ; assert(caught)
caught = true ; try { sfun(1, 2, 3, "string"); } catch { caught = true; } ; assert(caught)


//  Same but with Numbers

function nfun(a, b: Number, c: Number?, d: Number!): Number {
    return typeOf(a) + " " + typeOf(b) + " " + typeOf(c) + " " + typeOf(d)
}

assert(nfun(1, 2, 3, 4)                                          == "Number Number Number Number")
assert(nfun("1", "2", 3, 4)                                      == "String Number Number Number")
assert(nfun(null, null, null, 4)                                 == "Null Number Null Number")
assert(nfun(undefined, undefined, undefined, 4)                  == "Void Number Void Number")
assert(nfun(1, 2, 3, 4, 5, 6, 7)                                 == "Number Number Number Number")

//  Fail cases
caught = true ; try { nfun(); } catch { caught = true; } ; assert(caught)
caught = true ; try { nfun(null, null, null, null); } catch { caught = true; } ; assert(caught)
caught = true ; try { nfun(1, 2, 3, null); } catch { caught = true; } ; assert(caught)
*/
