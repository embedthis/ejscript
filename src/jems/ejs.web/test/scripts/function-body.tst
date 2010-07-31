/*
    Test function-body   
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/function-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - Function\n")
http.close()
