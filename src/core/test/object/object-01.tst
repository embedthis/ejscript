/*
  	Object tests
 */

o = new Object
assert(o != null)

o.a = 99
assert(o.a == 99)
assert(o.length == undefined)

b = new Object
assert(b != null)

b.address = "124 Park Lane"
assert(b.address = "124 Park Lane")

o.b = b
