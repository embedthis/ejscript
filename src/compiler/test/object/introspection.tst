/*
  	Test Object introspection
 */

assert(Object is Type)
assert(Object.getName(Object) == "Object")


//  Test Object

o = new Object
assert(o is Object)
assert(Object.getType(o) == Object)
assert(Object.getName(Object.getType(o)) == "Object")


//  Test scripted class

class Shape {}
assert(Shape is Type)
assert(Object.isType(Shape))
assert(Object.getName(Shape) == "Shape")
assert(Object.getType(Shape) == Type)
assert(Object.getBaseType(Shape) == Object)


//  Test class instance

s = new Shape
assert(Object.getType(s) == Shape)
assert(s is Object)


//  Test subclass
class Circle extends Shape {}
assert(Circle is Type)
assert(Object.getBaseType(Circle) == Shape)


//  Test subclass instance

c = new Circle
assert(c is Shape)
assert(c is Object)
assert(Object.getName(Object.getBaseType(Object.getType(c))) == "Shape")
assert(Object.getName(Object.getBaseType(Object.getBaseType(Object.getType(c)))) == "Object")

