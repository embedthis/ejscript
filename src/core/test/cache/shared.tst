/*
    Shared cach store tests
 */

require ejs.cache.local

//  Create a shared cache
cache1 = new Cache("local", {shared: true})
cache1.write("message", "Hello World")

cache2 = new Cache("local", {shared: true})
cache2.write("message", "Hello World 2")

assert(cache1.read("message") == "Hello World 2")
assert(cache2.read("message") == "Hello World 2")

