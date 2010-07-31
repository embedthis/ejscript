/*
    Test raw
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/raw.es")
assert(http.status == 200)
assert(http.response == "Hello World\r\n")
http.close()

