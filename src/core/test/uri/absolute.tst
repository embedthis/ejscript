/*
    Uri.absolute()
 */

assert(Uri("/a/b").absolute() == "http://localhost/a/b")

var base = Uri("http://example.com/a/b")
assert(Uri("/x/y").absolute(base) == "http://example.com/x/y")
assert(Uri("http://localhost/x/y").absolute(base) == "http://localhost/x/y")
assert(Uri("../d").absolute(base) == "http://example.com/a/d")
assert(Uri("/x/../y").absolute(base) == "http://example.com/y")
assert(Uri("x/../y").absolute(base) == "http://example.com/a/b/y")

//  Preserve trailing "/"
assert(Uri("x/y/").absolute(base) == "http://example.com/a/b/x/y/")

assert(Uri("").absolute(base) == "http://example.com/a/b")

//  Query must not inherit from base
base = Uri("http://example.com/a/b/c?d=e")
assert(Uri("/x/y/z").absolute(base) == "http://example.com/x/y/z")

//  Must not join with a relative base
var base = Uri("a/b/c")
assert(Uri("/x/y/z").absolute(base) == "http://localhost/x/y/z")

