/*
  	Test object methods
 */

user = { 
	name: "Peter",
	age: 27,
	color: "blue"
}

//	Test "clone"

peter = user.clone()
assert(peter != undefined)
assert("name" in peter)
assert(peter.name == "Peter")
assert(peter is Object)
assert(Object.getType(peter) == Object)


//	Test iterators: get/getValues

items = ["name", "age", "color"]
count = 0
for (key in peter) {
	assert(key == items[count])
	count++
}
assert(count == 3)

values = ["Peter", 27, "blue"]
count = 0
for each (value in peter) {
	assert(value == values[count])
	count++
}
assert(count == 3)


//	Test hashcode accessor

hash = hashcode(peter)
assert(hash is Number)
assert(hash != 0)


//	Test serialize

s = serialize(peter, { pretty: true })
assert(s == '{
  "name": "Peter",
  "age": 27,
  "color": "blue"
}')



//	deserialize

deserialize(s)


//	Test "type" and name accessors
class Shape {}
var s = new Shape
assert(s is Shape)
assert(Object.getType(s) == Shape)

