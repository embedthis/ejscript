/*
    Unit tests for Uri 
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
assert(u.port == 80)
assert(u.path == "/path/resource.gif")
assert(u.reference == "fragment")
assert(u.query == "ab=cd&ef=gh")
assert(u.extension == "gif")


//  Various accessors and parse
u = Uri({host: "www.example.com", query: "ab=cd&ef=gh"})
assert(!u.hasScheme)
assert(u.scheme == "http")
parts = u.components()
assert(parts.host == "www.example.com")
assert(parts.query == "ab=cd&ef=gh")

parts = Uri("http://www.example.com/path/to/resource.gif#fragment?abc=def&ghi=jkl").components()
assert(parts.scheme == "http")
assert(parts.host == "www.example.com")
assert(parts.port == null)
assert(parts.path == "/path/to/resource.gif")
assert(parts.reference == "fragment")
assert(parts.query == "abc=def&ghi=jkl")


//   Accessors
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


//  Join
u = Uri("/a/b").join("c")
assert(u.toString() == "/a/b/c")
assert(u == "/a/b/c")

u = Uri("/a/b").join("/c")
assert(u == "/c")

u = Uri("/a/b").join("../c")
assert(u == "/a/b/../c")


//  Normalize
u = Uri("/a/b").join("../c").normalize
assert(u == "/a/c")

u = Uri(".").normalize
assert(u == ".")
u = Uri("..").normalize
assert(u == "..")
u = Uri("../").normalize
assert(u == "..")
u = Uri("../abc").normalize
assert(u == "../abc")
u = Uri("/top/mid/../file").normalize
assert(u == "/top/file")


//  Basename
u = Uri("/")
assert(u.basename == "")
u = Uri("/a")
assert(u.basename == "a")
u = Uri("/a/")
assert(u.basename == "")
u = Uri("/a/b")
assert(u.basename == "b")


//  Dirname
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


//  Relative
u = Uri("/a/b/c")
assert(u.relative("/") == "../..")
assert(u.relative("/other/path/def") == "../../other/path/def")
assert(u.relative("/a/b/c/d/e/f") == "d/e/f")


//  Complete
u = Uri({ path: "/a/b/c"})
assert(u.complete == "http://localhost/a/b/c")

u = Uri({ scheme: "https", path: "/a/b/c"})
assert(u.complete == "https://localhost/a/b/c")

u = Uri({ host: "www.example.com", path: "/a/b/c"})
assert(u.complete == "http://www.example.com/a/b/c")

u = Uri({ host: "www.example.com", path: "/a/b/c", reference: "extra", query: "priority=high"})
assert(u.complete == "http://www.example.com/a/b/c#extra?priority=high")


//  Encode
s = Uri.encode("http://www.example.com/a/b/c#extra?priority=high&urgency=extreme")
assert(s == "http%3a%2f%2fwww.example.com%2fa%2fb%2fc%23extra%3fpriority%3dhigh%26urgency%3dextreme")
s = Uri.decode("http%3a%2f%2fwww.example.com%2fa%2fb%2fc%23extra%3fpriority%3dhigh%26urgency%3dextreme")
assert(s == "http://www.example.com/a/b/c#extra?priority=high&urgency=extreme")


//  Setters
u = Uri("http://www.example.com/path#ref?pri=high")
u.path = "short/path"
assert(u == "http://www.example.com/short/path#ref?pri=high")

assert(u.scheme == "http")
u.scheme = "https"
assert(u.scheme == "https")

assert(u.host == "www.example.com")
u.host = "example.net"
assert(u.host == "example.net")

assert(u.port == 443)
u.port = 4443
assert(u.port == 4443)
assert(u == "https://example.net:4443/short/path#ref?pri=high")

u.uri = "short/path"
assert(u == "short/path")
assert(u.path == "short/path")
assert(u.host == "localhost")
assert(u.port == 80)


//  Trim
u = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u.trimEnd(".gif").path == "/short/path")
assert(u.trimStart("/short").path == "/path.gif")


//  Mime
u = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u.mimeType == "image/gif")
assert(Uri("a.txt").mimeType == "text/plain")
assert(Uri("a.html").mimeType == "text/html")
assert(Uri("a.json").mimeType == "application/json")


//  JoinExt, ReplaceExt, TrimExt
u = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u.extension == "gif")
u = u.trimExt("gif")
assert(u.path == "/short/path")
u = u.joinExt(".jpeg")
assert(u.path == "/short/path.jpeg")
assert(u.extension == "jpeg")
u = u.replaceExt(".png")
assert(u.path == "/short/path.png")
assert(u.extension == "png")


//  Comparison via same()
u1 = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
u2 = Uri("http://example.net:4443/short/path.gif#ref?pri=high")
assert(u1 != u2)

u1 = Uri("https://example.net/short/path.gif#ref?pri=high")
u2 = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u1 != u2)

u1 = Uri("http://www.example.com/short/path.gif#ref?pri=high")
u2 = Uri("http://www.example.com/short/path.gif?pri=high")
assert(u1 != u2)

u1 = Uri("http://www.example.com/short/path.gif?pri=high")
u2 = Uri("http://www.example.com/short/path.gif")
assert(u1 != u2)


/* TODO BUG
u = Uri("http://www.example.com/short/path.gif?pri=high")
assert(u.toJSON() == "http://www.example.com/short/path.gif?pri=high")

u = Uri("http://www.embedthis.com/index.html")
print(u.readString())

function readBytes(): ByteArray
function readLines(): Array
function readXML(): XML
*/

/*
    Various corner cases
 */
u = Uri("www.example.com:7000")
o = u.components()
assert(o.host = "www.example.com")
assert(o.port = 7000)
assert(o.path = "/")

