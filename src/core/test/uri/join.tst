/*
    Uri join tests
 */

//  Join

u = Uri("/a").join("b")
assert(u == "/a/b")

u = Uri("a").join("b")
assert(u == "a/b")

u = Uri("/a/").join("b")
assert(u == "/a/b")

//  Not normalized
u = Uri("/a/./").join("b")
assert(u == "/a/./b")
assert(u.normalize == "/a/b")


u = Uri("/a/b/").join("c")
assert(u.toString() == "/a/b/c")
assert(u == "/a/b/c")

u = Uri("/a/b").join("c")
assert(u == "/a/b/c")

u = Uri("/a/b").join("/c")
assert(u == "/c")

u = Uri("/a/b/c").join("../d").normalize
assert(u == "/a/b/d")

u = Uri("/a/b/c").join("http://localhost/abc")
assert(u == "http://localhost/abc")

u = Uri("/a/b").join("../c").normalize
assert(u == "/a/c")
