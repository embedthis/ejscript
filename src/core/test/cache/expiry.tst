/*
    Expiry tests
 */

//  Test expiry
cache = new Cache("local", {resolution: 100})
cache.write("message", "Hello World", {lifespan: 1})
assert(cache.read("message") == "Hello World")
App.sleep(1100)
assert(cache.read("message") == null)
