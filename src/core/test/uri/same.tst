/*
    Uri.same()
 */

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

