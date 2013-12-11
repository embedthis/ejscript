/*
    form.tst -- Test a form and post-back
 */

const HTTP = App.config.uris.http

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/form.ejs")
assert(http.status == 200)
assert(http.response.contains("<form"))
assert(!http.response.contains("Form accepted."))
http.close()

//  Test POST to the form
http.form(HTTP + "/form.ejs", {name: "Tom", address: "700 Park Ave"})
assert(http.status == 200)
let result = http.response
assert(!result.contains("<form"))
assert(result.contains("Form accepted."))
assert(result.contains("Name: Tom, Address: 700 Park Ave"))
http.close()
