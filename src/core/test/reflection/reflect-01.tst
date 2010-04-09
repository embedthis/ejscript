/*
 *	Test reflection
 */

assert(Object is Type)
assert(Reflect(Object).name == "Object")


//  Test Object

o = new Object
assert(o is Object)
assert(Reflect(o).type == Object)
assert(Reflect(o).base == Object)
assert(Reflect(Reflect(o).type).name == "Object")


//  Test scripted class

class Shape {}
assert(Shape is Type)
assert(Reflect(Shape).isType)
assert(Reflect(Shape).name == "Shape")
assert(Reflect(Shape).type == Type)
assert(Reflect(Shape).base == Object)


//  Test class instance

s = new Shape
assert(Reflect(s).base == Shape)
assert(s is Object)


//  Test subclass
class Circle extends Shape {}
assert(Circle is Type)
assert(Reflect(Shape).base == Object)


//  Test subclass instance

c = new Circle
assert(c is Shape)
assert(c is Object)
assert(Reflect(Reflect(Reflect(c).type).base).name == "Shape")
assert(Reflect(Reflect(Reflect(Reflect(c).type).base).base).name == "Object")

