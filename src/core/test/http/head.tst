/*
    Head tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.head(HTTP + "/index.html")
assert(http.status == 200)
assert(http.response == "")
assert(http.contentLength > 0) 
http.close()
