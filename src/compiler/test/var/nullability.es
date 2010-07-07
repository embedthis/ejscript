/*
    Test nullability type operators in variable declarations
 */

//  Explicit allow null/undefine (this is the default)
var x: String?
x = "abc"       ; assert(typeOf(x) == "String")
x = null        ; assert(x === null)
x = undefined   ; assert(x === undefined)
x = 77          ; assert(x is String && x == "77")
x = Object      ; assert(x is String && x == "[object Type]")


//  Prohibit null/undefine 
var y: String!
y = "abc"       ; assert(typeOf(y) == "String")
var caught = false
try {
    y = null        ; assert(y === null)
} catch { caught = true }
assert(caught)

var caught = false
try {
    y = undefined   ; assert(y === undefined)
} catch { caught = true }
assert(caught)

y = 77          ; assert(y is String && y == "77")
y = Object      ; assert(y is String && y == "[object Type]")
