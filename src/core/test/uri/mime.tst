/*
    Uri.mimeType
 */

u = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u.mimeType == "image/gif")
assert(Uri("a.txt").mimeType == "text/plain")
assert(Uri("a.html").mimeType == "text/html")
assert(Uri("a.json").mimeType == "application/json")
