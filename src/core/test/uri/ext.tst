/*
    Uri.joinExt, replaceExt, trimExt
 */

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
