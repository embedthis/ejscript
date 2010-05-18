/*
    Test own names and keys
 */

function contains(o, elements) {
    for each (item in elements) {
        assert(o.contains(item))
    }
}


//  MOB BUG -- should contain "length"
contains(Object.getOwnPropertyNames(Array), ["prototype"])

contains(Object.getOwnPropertyNames(Boolean), ["prototype"])
contains(Object.getOwnPropertyNames(Date), ["UTC", "now", "parse", "parseDate", "parseUTCDate", "prototype"])
contains(Object.getOwnPropertyNames(Error), ["prototype"])
contains(Object.getOwnPropertyNames(Function), ["prototype"])
contains(Object.getOwnPropertyNames(Number), ["MAX_VALUE", "MIN_VALUE", "MaxValue", "MinValue", "NEGATIVE_INFINITY", 
        "NaN", "POSITIVE_INFINITY", "prototype"])
contains(Object.getOwnPropertyNames(Object), ["create", "defineProperties", "defineProperty", "freeze", 
        "getOwnPropertyCount", "getOwnPropertyDescriptor", "getOwnPropertyNames", "getOwnPrototypeOf", 
        "isExtensible", "isFrozen", "isSealed", "keys", "preventExtensions", "prototype", "seal"])
contains(Object.getOwnPropertyNames(String), ["fromCharCode", "prototype"])


//  keys for key types. These are the enumerable properties and should match for/in.

assert(Object.keys(Array) == "")
assert(Object.keys(Boolean) == "")
assert(Object.keys(Date) == "")
assert(Object.keys(Error) == "")
assert(Object.keys(Function) == "")
assert(Object.keys(Number) == "")
assert(Object.keys(Object) == "")
assert(Object.keys(String) == "")


//  Test constructor property

o = {}
assert(o.constructor == Object)
i = 7
assert(i.constructor == Number)
assert(Date().constructor == Date)

