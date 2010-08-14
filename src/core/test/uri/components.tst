/*
    Uri.components
 */

parts = Uri("http://www.example.com/path/to/resource.gif#fragment?abc=def&ghi=jkl").components
assert(parts.scheme == "http")
assert(parts.host == "www.example.com")
assert(parts.port == null)
assert(parts.path == "/path/to/resource.gif")
assert(parts.reference == "fragment")
assert(parts.query == "abc=def&ghi=jkl")

//  With port and without path 
u = Uri("www.example.com:7000")
parts = u.components
assert(parts.host = "www.example.com")
assert(parts.port = 7000)
assert(parts.path = "/")
