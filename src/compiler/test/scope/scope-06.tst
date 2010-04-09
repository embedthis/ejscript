/*
    MOB - incomplete
 */


//  Instance methods are visible to instances

var i = new String
assert(i.charAt)
assert(i.fromCharCode == undefined)

//  Instance methods should not be visible from types

var s = String
assert(s.charAt == undefined)
assert(s.fromCharCode)

//  Test visibility of an instance method defined in a base class

assert(i.hasOwnProperty)
assert(s.hasOwnProperty)
assert(String.hasOwnProperty)
assert(Object.hasOwnProperty)

assert(i.isFrozen == undefined)
assert(s.isFrozen)
assert(String.isFrozen)
assert(Object.isFrozen)
