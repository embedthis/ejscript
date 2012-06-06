/*
    Test raw
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.get(HTTP + "/raw.es")
assert(http.status == 200)
assert(http.response == "Hello World\r\n")
http.close()

