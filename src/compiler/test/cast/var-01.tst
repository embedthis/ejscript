/*
    Test variable declaration typing
 */

var n: Number
n = 77
assert(n is Number)
n = "1234"
assert(n is Number)
n = true
assert(n is Number)
n = null
assert(n is Null)
n = {}
assert(n is Number)


var s: String
s = "hello"
assert(s == "hello")
assert(s is String)
s = 77
assert(s == "77")
assert(s is String)
s = null
assert(s == null)
assert(s is Null)

var sn: String!
sn = "hello"
assert(sn == "hello")
assert(sn is String)
sn = 77
assert(sn == "77")
assert(sn is String)

caught = false ; try { sn = null; } catch { caught = true; } ; assert(caught)
assert(sn == "77")

