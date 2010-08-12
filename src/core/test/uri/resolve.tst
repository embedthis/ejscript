/*
    Uri.resolve()
 */

base = Uri("/a/b/c.html")
assert(base.resolve("") == "/a/b")
assert(base.resolve("d.html") == "/a/b/d.html")
assert(base.resolve("/") == "/")
assert(base.resolve("..").normalize == "/a/")

base = Uri("c.html")
assert(base.resolve("") == "")
assert(base.resolve("/") == "/")
assert(base.resolve("a.html") == "a.html")
