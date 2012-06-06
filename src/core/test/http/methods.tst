/*
    Method tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.connect("OPTIONS", HTTP + "/index.html")
assert(http.header("Allow") == "OPTIONS,GET,HEAD,POST,PUT,DELETE")
http.close()

http.connect("TRACE", HTTP + "/index.html")
assert(http.status == 406)
http.close()
