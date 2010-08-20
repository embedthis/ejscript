/*
    Uri.relative()
 */

base = Uri("/a/b/c")
assert(Uri("/").relative(base) == "../..")
assert(Uri("/other/path/def").relative(base) == "../../other/path/def")
assert(Uri("/a/b/c/d/e/f").relative(base) == "d/e/f")

assert(Uri("/a/b.html").relative("some/relative/path") == "/a/b.html")
assert(Uri("c.html").relative("/a/b.html") == "c.html")

assert(Uri("").relative(base) == "")
assert(Uri("..").relative(base) == "..")
assert(Uri("../").relative(base) == "../")
assert(Uri("../..").relative(base) == "../..")

base = Uri("http://localhost/a/b/c?x=y")
assert(Uri("/a/b/c/d/e/f").relative(base) == "d/e/f")
assert(Uri("/a").relative(base) == "../..")

//  Test trailing slashes
assert(Uri("/abc").relative("/abc/xyz") == "..")
assert(Uri("/abc/").relative("/abc/xyz") == ".")
assert(Uri("/abc").relative("/abc/xyz/") == "../..")

assert(Uri("/abc/def").relative("/xyz") == "abc/def")
