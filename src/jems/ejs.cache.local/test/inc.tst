/*
    Inc tests
 */

require ejs.cache.local

cache = new LocalCache

//  Write a simple value and increment
cache.write("num", 0)
assert(cache.read("num") == 0)
cache.inc("num")
assert(cache.read("num") == 1)

//  Increment again
assert(cache.inc("num") == 2)
assert(cache.inc("num") == 3)
assert(cache.inc("num") == 4)

//  Increment by twos
assert(cache.inc("num", 2) == 6)
assert(cache.inc("num", 2) == 8)

//  Decrement
assert(cache.inc("num", -1) == 7)
assert(cache.inc("num", -1) == 6)

