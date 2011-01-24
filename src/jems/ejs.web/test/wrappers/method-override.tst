/*
    Test MethodOverride wrapper
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Override the POST method to be DELETE using params
http.form(HTTP + "/dispatch.es", { route: "override", "-ejs-method-": "DELETE"})
let response = deserialize(http.response)
assert(response.originalMethod == "POST")
assert(response.method == "DELETE")
assert(response.params.route == "override")
http.close()

//  Override the POST method to be DELETE using X-HTTP-METHOD-OVERRIDE
http.setHeader("X-HTTP-METHOD-OVERRIDE", "DELETE")
http.form(HTTP + "/dispatch.es", { route: "override" })
let response = deserialize(http.response)
assert(response.originalMethod == "POST")
assert(response.method == "DELETE")
assert(response.params.route == "override")
http.close()
