/*
    Uri.relative()
 */

base = Uri("/a/b/c")
assert(Uri("/").relative(base) == "../..")
assert(Uri("/other/path/def").relative(base) == "../../other/path/def")

//  Note: images is not a dir
assert(Uri("/web/images").relative("/web") == "web/images")

assert(Uri("/a/b.html").relative("some/relative/path") == "/a/b.html")
assert(Uri("c.html").relative("/a/b.html") == "c.html")

assert(Uri("").relative(base) == "")
assert(Uri("..").relative(base) == "..")
assert(Uri("../").relative(base) == "../")
assert(Uri("../..").relative(base) == "../..")

base = Uri("http://localhost/a/b/c?x=y")
assert(Uri("/a/b/c/d/e/f").relative(base) == "c/d/e/f")
assert(Uri("/a").relative(base) == "../../a")

//  Test trailing slashes
assert(Uri("/abc").relative("/abc/xyz") == "../abc")
assert(Uri("/abc/").relative("/abc/xyz") == ".")
assert(Uri("/abc").relative("/abc/xyz/") == "../../abc")

assert(Uri("/abc/def").relative("/xyz") == "abc/def")
