/*
    XML tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.get(HTTP + "/test.xml")
assert(http.readXml().customer.name == "Joe Green")
http.close()
