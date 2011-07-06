/*
    Multiple cache store tests
 */

//  Create cache and write
cache1 = new Cache("local")
cache1.write("message", "Hello World")
assert(cache1.read("message") == "Hello World")


//  Test a second cache and check values don't interfere
cache2 = new Cache("local", {shared: false})
assert(cache1.read("message") == "Hello World")
cache2.write("message", "Hello World 2")
assert(cache2.read("message") == "Hello World 2")
assert(cache1.read("message") == "Hello World")
cache1.destroy()
cache2.destroy()
