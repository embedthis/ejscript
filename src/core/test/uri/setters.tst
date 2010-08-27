/*
    Setter properties Uri 
 */

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
assert(u.host === null)
assert(u.port === null)

