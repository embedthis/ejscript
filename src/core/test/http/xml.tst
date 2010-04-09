/*
    XML tests
 */


const HTTP = (global.session && session["http"]) || ":6700"
var http: Http = new Http

http.get(HTTP + "/test.xml")
assert(http.readXml().customer.name == "Joe Green")
http.close()
