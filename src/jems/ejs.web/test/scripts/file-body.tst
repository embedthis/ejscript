/*
    Test file-body   
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.get(HTTP + "/file-body.es")
assert(http.status == 200)
assert(http.response == "Hello World - File\n")
http.close()
