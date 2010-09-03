/*
    Uri.template
 */

assert(Uri.template("") == "")
assert(Uri.template("/") == "/")
assert(Uri.template("/anything") == "/anything")

assert(Uri.template("{keyword}", {keyword: "abc"}) == "abc")
assert(Uri.template("/{keyword}", {keyword: "abc"}) == "/abc")
assert(Uri.template("{keyword}/", {keyword: "abc"}) == "abc/")

o = { action: "login", id: 77 }
r = { controller: "User" }
assert(Uri.template("/Dash/{controller}/inter/{action}/{id}", o, r) == "/Dash/User/inter/login/77")
