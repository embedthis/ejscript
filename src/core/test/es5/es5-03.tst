/*
    Test own names and keys
 */

print(Object.getOwnPropertyNames(Array.prototype).sort())

/*
print(Object.getOwnPropertyNames(Array).sort())
print(Object.getOwnPropertyNames(Date).sort())
print(Object.getOwnPropertyNames(Number).sort())
print("----")
print("----")
print(Array.length)
*/

/*
assert(Object.getOwnPropertyNames(Array).sort() == "length,prototype")
assert(Object.getOwnPropertyNames(Boolean).sort() == "length,prototype")
assert(Object.getOwnPropertyNames(Date).sort() == "UTC,length,now,parse,parseDate,parseUTCDate,prototype")
assert(Object.getOwnPropertyNames(Error).sort() == "length,prototype")
assert(Object.getOwnPropertyNames(Function).sort() == "length,prototype")
assert(Object.getOwnPropertyNames(Number).sort() == "MAX_VALUE,MIN_VALUE,MaxValue,MinValue,NEGATIVE_INFINITY,NaN,POSITIVE_INFINITY,length,prototype")
assert(Object.getOwnPropertyNames(Object).sort() == "create,defineProperties,defineProperty,freeze,getOwnPropertyCount,getOwnPropertyDescriptor,getOwnPropertyNames,getOwnPrototypeOf,isExtensible,isFrozen,isSealed,keys,length,preventExtensions,prototype,seal")
assert(Object.getOwnPropertyNames(String).sort() == "fromCharCode,length,prototype")


//  keys for key types. These are the enumerable properties and should match for/in.

assert(Object.keys(Array).sort() == "")
assert(Object.keys(Boolean).sort() == "")
assert(Object.keys(Date).sort() == "")
assert(Object.keys(Error).sort() == "")
assert(Object.keys(Function).sort() == "")
assert(Object.keys(Number).sort() == "")
assert(Object.keys(Object).sort() == "")
assert(Object.keys(String).sort() == "")
*/
