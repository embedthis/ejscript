/*
    Test file-body   
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/file-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - File\n")
http.close()
