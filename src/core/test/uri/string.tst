/*
    Uri.toString()
 */

u = Uri({ scheme: "http", host: "localhost", port: 8080, path: "/dir/index.html", query: "a=b&c=d", reference: "section"})
assert(u == "http://localhost:8080/dir/index.html#section?a=b&c=d")
assert(u.toString() == "http://localhost:8080/dir/index.html#section?a=b&c=d")
assert(u.toLocalString() == "/dir/index.html#section?a=b&c=d")


u = Uri({ host: "localhost", port: 8080, path: "index.html"})
assert(u.toString() == 'localhost:8080/index.html')
