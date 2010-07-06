/*
    Destructuring assignment
 */

// Simple array de-assign
var x = [1,2]
[a, b] = x
assert(a == 1)
assert(b == 2)

// Array swap
var a = 1, b = 2
[a,b] = [b,a]
assert(a == 2)
assert(b == 1)

// Array de-assign with gaps
var x = [1,2,3,4]
[a, b, , d] = x
assert(a == 1)
assert(b == 2)
assert(d == 4)

// Var declarations via de-assign
var x = [1,2,3,4]
var [a, b, , d] = x
assert(a == 1)
assert(d == 4)

// Object literal without colons
var priority = "urgent"
obj = {priority}
assert(obj.priority == "urgent")

// Object literal without colons with literal
obj = {"priority"}
assert(obj.priority == null)
assert(Object.getOwnPropertyCount(obj) == 1)

// Object de-assign
var obj = { name: "joe", phone: "1234567", zip: 98100}
({ name: a, phone: b }) = obj
assert(a == "joe")
assert(b == "1234567")

// Object de-assign without colons
var obj = { name: "joe", phone: "24681234", zip: 98100}
({ name, phone }) = obj
assert(name == "joe")
assert(phone == "24681234")

//  Empty assignment (makes parsers easier)
var obj = {something: 43}
({}) = obj

//  Object de-assign with empties
var obj = { name: "mary", phone: "124681234", zip: 98100}
({ name, , zip}) = obj
assert(name == "mary")
assert(zip == 98100)

//  Nested array de-assignment
[a,[b,c]] = [1,[2,3]]
assert(a == 1)
assert(b == 2)
assert(c == 3)

// More complex nesting
function f() { return [1, 2, [10, 20, [30, 40]]] }
[a,,[b,,[c]]] = f();
assert(a == 1)
assert(b == 10)
assert(c == 30)

// Nested object de-assign
o = { kind: "user", details: { name: "joe", phone: "1234567" }}
({kind}) = o
assert(kind == "user")

//  Nested object deassign
o = { kind: "user", details: { name: "joe", phone: "1234567" }}
// ({kind, {name}}) = o
({kind: kind, details: {name}}) = o
assert(kind == "user")
assert(name == "joe")

//  Iterate over an object using key/value pair
let keys = []
let values = []
obj = { color: "red", flavor: "spicy", temp: "hot" }
for (let [key, value] in obj) {
    keys += [key]
    values += [value]
}
assert(keys == "color,flavor,temp")
assert(values == "red,spicy,hot")

// Iterate over an array using key/value pair
let keys = []
let values = []
for (var [key,value] in [10,11,12]) {
    keys += [key]
    values += [value]
}
assert(keys == "0,1,2")
assert(values == "10,11,12")
