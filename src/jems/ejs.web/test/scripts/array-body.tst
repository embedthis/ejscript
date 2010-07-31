/*
    Test array-body   
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/array-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - Array\n")
http.close()

