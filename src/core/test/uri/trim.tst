/*
    Uri.trimEnd, Uri.trimStart
 */

u = Uri("https://example.net:4443/short/path.gif#ref?pri=high")
assert(u.trimEnd(".gif").path == "/short/path")
assert(u.trimStart("/short").path == "/path.gif")
