/*
    Post tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.post(HTTP + "/index.html", "Some data")
assert(http.status == 200)
http.close()

