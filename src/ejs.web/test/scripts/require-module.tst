/*
    Test require-module
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.get(HTTP + "/require-module.es")
assert(http.status == 200)
assert(http.response == "Hello World - Custom Header\n")
assert(http.header("Custom-Header") == "Custom Data")
http.close()

