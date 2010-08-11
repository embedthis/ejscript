/*
    Uri.dirname
 */

u = Uri("/")
assert(u.dirname == "/")
u = Uri("/a")
assert(u.dirname == "/")
u = Uri("/a/")
assert(u.dirname == "/a")
u = Uri("/a/b")
assert(u.dirname == "/a")
u = Uri("/a/b/")
assert(u.dirname == "/a/b")

