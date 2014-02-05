print(Object.getOwnPropertyNames(Boolean))
print()
print(Object.getOwnPropertyNames(Boolean.prototype))
print()


function Shape(height, width) {
    this.height = height
    this.width = width
    return this
}

// breakpoint()
Shape.prototype.marker = "Hello"
Shape.prototype.area = function() {
    return this.height * this.width
}
s = new Shape(1,2)

//  arguments,callee,caller,length,name,prototype       current: prototype
print(Object.getOwnPropertyNames(Shape))
print()

//  constructor,marker,area             current: marker, area
print(Object.getOwnPropertyNames(Shape.prototype))

/*
function Shape(height, width) {
    this.height = height
    this.width = width
    return this
}
Shape.prototype.marker = "Hello"
Shape.prototype.area = function() {
    return this.height * this.width
}
s = new Shape(1,2)
// print(Reflect(s.constructor).name)

function Cube(d) {
    this.depth = d
}
Cube.prototype = new Shape(1, 2)

print(Cube.prototype.constructor)
print(Reflect(Cube.prototype.constructor).name)
print(Cube)
// assert(Cube.prototype.constructor == Shape)

//  Differences
//      - constructor is a getter
//      - constructor result is a Type/Function not just a function
//      - FAILS: because Shape is function here assert(Cube.prototype.constructor == Shape)


// assert(Cube.prototype.constructor === Cube)
// Cube.prototype.constructor = Cube

Cube.prototype.toString = function() {
    //  TODO -- this is not being called
    print("TOSTRING")
}

c = new Cube(3)
print(Reflect(c.constructor).name)
print(c.constructor)
print("" + c)


*/
