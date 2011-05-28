/*
    Create tests
 */

//  Create cache and write
cache = new Cache
cache.write("message", "Hello World")
assert(cache.read("message") == "Hello World")
cache.destroy()
