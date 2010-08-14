/*
    Uri.basename
 */

u = Uri("/a/b/c")
assert(u.basename == "c")
u = Uri("/")
assert(u.basename == "")
u = Uri("/a")
assert(u.basename == "a")
u = Uri("/a/")
assert(u.basename == "")
u = Uri("/a/b")
assert(u.basename == "b")
