/*
    LocalCache tests
 */

require ejs.cache.local

//  Create cache and write
l1 = new LocalCache
l1.write("message", "Hello World")
assert(l1.read("message") == "Hello World")


//  Test a second cache and check values don't interfere
l2 = new LocalCache
l2.write("message", "Hello World 2")
assert(l2.read("message") == "Hello World 2")
assert(l1.read("message") == "Hello World")


//  Create a shared cache
l1 = new LocalCache({shared: true})
l1.write("message", "Hello World")

l2 = new LocalCache({shared: true})
l2.write("message", "Hello World 2")

assert(l1.read("message") == "Hello World 2")
assert(l2.read("message") == "Hello World 2")


//  Test expiry
l = new LocalCache({resolution: 100})
l.write("message", "Hello World", {lifespan: 1})
assert(l.read("message") == "Hello World")
App.sleep(300)
assert(l.read("message") == null)
