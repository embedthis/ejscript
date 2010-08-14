/*
    Uri.complete()
 */

u = Uri({ path: "/a/b/c"})
assert(u.complete() == "http://localhost/a/b/c")

u = Uri({ scheme: "https", path: "/a/b/c"})
assert(u.complete() == "https://localhost/a/b/c")

u = Uri({ host: "www.example.com", path: "/a/b/c"})
assert(u.complete() == "http://www.example.com/a/b/c")

u = Uri({ host: "www.example.com", path: "/a/b/c", reference: "extra", query: "priority=high"})
assert(u.complete() == "http://www.example.com/a/b/c#extra?priority=high")



u = Uri({ path: "/a/b/c"})
assert(u.complete("http://example.com") == "http://example.com/a/b/c")

u = Uri({ path: "/a/b/c"})
assert(u.complete({ scheme: "https", port: 7777}) == "https://localhost:7777/a/b/c")

u = Uri({ path: "/a/b/c"})
assert(u.complete(Uri("http://example.com")) == "http://example.com/a/b/c")

//  Relative 
u = Uri({ path: "a/b/c"})
assert(u.complete(Uri("http://example.com")) == "http://example.com/a/b/c")

