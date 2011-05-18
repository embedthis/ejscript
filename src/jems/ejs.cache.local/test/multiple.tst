/*
    Multiple cache store tests
 */

require ejs.cache.local

//  Create cache and write
cache1 = new LocalCache
cache1.write("message", "Hello World")
assert(cache1.read("message") == "Hello World")


//  Test a second cache and check values don't interfere
cache2 = new LocalCache
cache2.write("message", "Hello World 2")
assert(cache2.read("message") == "Hello World 2")
assert(cache1.read("message") == "Hello World")
cache1.destroy()
cache2.destroy()
