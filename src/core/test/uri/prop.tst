/*
    Getter properties for Uri
 */

u = Uri("http://www.example.com/short/path.gif")
assert(u.endsWith("path.gif"))
assert(u.startsWith("/short"))
assert(u.hasScheme)
assert(u.hasHost)
assert(!u.hasPort)
assert(u.hasExtension)
assert(!u.hasPort)
assert(!u.hasReference)
assert(!u.hasQuery)
assert(u.isAbsolute)
assert(!u.isRelative)
assert(!u.isDir)
assert(u.isRegular)


//  With query
u = Uri({host: "www.example.com", query: "ab=cd&ef=gh"})
assert(!u.hasScheme)
assert(u.scheme == "http")
parts = u.components
assert(parts.host == "www.example.com")
assert(parts.query == "ab=cd&ef=gh")

