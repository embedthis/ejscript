/*
    Test string-body   
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.get(HTTP + "/string-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - String\n")
http.close()
