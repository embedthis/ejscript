/*
    Test require-module
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/require-module.es")
assert(http.status == 200)
assert(http.response == "Hello World - Custom Header\n")
assert(http.headers["custom-header"] == "Custom Data")
http.close()

