/*
    Uri join tests
 */


//  Join
u = Uri("/a/b/").join("c")
assert(u.toString() == "/a/b/c")
assert(u == "/a/b/c")

u = Uri("/a/b").join("c")
assert(u.toString() == "/a/c")
assert(u == "/a/c")

u = Uri("/a/b").join("/c")
assert(u == "/c")

u = Uri("/a/b/c").join("../d")
assert(u == "/a/d")

u = Uri("/a/b/c").join("http://localhost/abc")
assert(u == "http://localhost/abc")

u = Uri("/a/b").join("../c")
assert(u == "/c")

