/*
    Test function-body   
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.get(HTTP + "/function-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - Function\n")
http.close()
