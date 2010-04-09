/*
    Test prototype based inheritance
 */
// load("Compat.es")

function MyShape(x, y) {
    this.x = x
    this.y = y
}

// new MyShape(1, 1)
// print("OWN " + Object.getOwnPropertyNames(MyShape))
// print("PROTOTYPE " + MyShape.prototype)
// MyShape.z = 7
// dump(MyShape)

MyShape.prototype.z = 7

m = new MyShape(1, 4)
dump(m)
print(m.z)



/*
var m = new MyShape
breakpoint()
print(MyShape.prototype)
*/

// assert(typeOf(String.prototype) == "Object")
// dump(String.prototype)


/*
var s = new MyShape(2, 2)
assert(s)
assert(s.width == 2)
assert(s.height == 2)

print("KEYS " + Object.keys(MyShape))
dump(s)
print("DONE")
*/



/*
MyShape.abc = 7

dump("MyShape", MyShape)
print("AFTER")
for (k in MyShape) {
    print(k)
}
*/

// dump(MyShape)

/*
s = new MyShape(100, 200)
breakpoint()
print(MyShape.prototype)
assert(Object.getOwnPrototypeOf(s) === MyShape.prototype)
*/

/*
assert(MyShape.prototype.isPrototypeOf(s))
proto = MyShape.prototype
proto.marker = "Hello"
proto.area = function() {
    return this.height * this.width
}
assert(s.marker == "Hello")
assert(s.area() == 20000)


//  Create another shape to ensure prototype is working

s2 = new MyShape(1, 2)
assert(s.area() == 20000)
assert(s2.area() == 2)


//  create

s3 = Object.create(MyShape.prototype, {height: {value: 5}, width: {value: 2}})
assert(s3.area() == 10)
*/
