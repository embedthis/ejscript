
var i = new String

var s = String

/*
print(i.isFrozen)
print(s.isFrozen)
print(String.isExtensible)
print(Object.isExtensible)
print(Object.keys)
print(Object.preventExtensions)
print(Object.getOwnPropertyDescriptor)
*/

print(Object.constructor)                   //  Y
print(Object.prototype)                     //  Y
print(Object.create)                        //  Y
print(Object.defineProperty)                //  Y
print(Object.defineProperties)              //  Y
print(Object.getOwnPropertyDescriptor)      //  Y
print(Object.getOwnPropertyNames)           //  Y
print(Object.keys)                          //  Y
    
print(String.constructor)                   //  Y
print(String.prototype)                     //  Y
print(String.create)                        //  Y
print(String.defineProperty)                //  Y
print(String.defineProperties)              //  Y
print(String.getOwnPropertyDescriptor)      //  Y
print(String.getOwnPropertyNames)           //  Y
print(String.keys)                          //  Y
    
/*  JSC not defined
print(Object.freeze)
print(Object.getOwnPrototypeOf)
print(Object.isExtensible)
print(Object.isFrozen)
print(Object.isSealed)
print(Object.preventExtensions)
print(Object.seal)
*/
