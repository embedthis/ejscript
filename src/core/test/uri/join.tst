/*
    Uri.join()
 */

u = Uri("/a").join("b")
assert(u == "/a/b")

u = Uri("a").join("b")
assert(u == "a/b")

u = Uri("/a/").join("b")
assert(u == "/a/b")

u = Uri("/a/b").join("")
assert(u == "/a/b")

//  Not normalized
u = Uri("/a/./").join("b")
assert(u == "/a/./b")
assert(u.normalize == "/a/b")

u = Uri("/a/b/").join("c")
assert(u == "/a/b/c")

u = Uri("/a/b").join("c")
assert(u == "/a/b/c")

//  Join absolute
u = Uri("/a/b").join("/c")
assert(u == "/c")
u = Uri("/a/b").join("http://localhost/c")
assert(u == "http://localhost/c")

u = Uri("/a/b/c").join("../d").normalize
assert(u == "/a/b/d")

u = Uri("/a/b/c").join("http://localhost/abc")
assert(u == "http://localhost/abc")

u = Uri("/a/b").join("../c").normalize
assert(u == "/a/c")

//  Join multiple
u = Uri("/a").join("b", "c", "d")
assert(u == "/a/b/c/d")

//  Join different arg types

u = Uri("/a").join(Path("b"), Uri("c"), {path: "d"})
assert(u == "/a/b/c/d")

u = Uri("/a").join(Path("b"), {scheme: "http", path: "/d"})
assert(u == "http:///d")

u = Uri("/a").join(Path("b"), {scheme: "http", path: "/d"})
assert(u == "http:///d")
