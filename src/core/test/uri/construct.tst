/*
    Uri constructors
 */

//  Constructor from string
p = Path("/")
u = Uri("http://www.example.com/path/to/resource#fragment?ab=cd&ef=gh")
assert(u == "http://www.example.com/path/to/resource#fragment?ab=cd&ef=gh")


//  Constructor from hash
u = Uri({scheme: "http", host: "www.example.com", path: "/path/resource.gif", reference: "fragment", query: "ab=cd&ef=gh"})
assert(u == "http://www.example.com/path/resource.gif#fragment?ab=cd&ef=gh")
assert(u.scheme == "http")
assert(u.host == "www.example.com")
assert(!u.hasPort)
assert(u.port == null)
assert(u.path == "/path/resource.gif")
assert(u.reference == "fragment")
assert(u.query == "ab=cd&ef=gh")
assert(u.extension == "gif")


//  Various uri values
u = new Uri("")
assert(u == "")

u = new Uri("/")
assert(u == "/")

u = new Uri("../")
assert(u == "../")

u = new Uri("/a/b/c")
assert(u.basename == "c")
assert(u.dirname == "/a/b")
