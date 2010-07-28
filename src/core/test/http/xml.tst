/*
    XML tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/test.xml")
http.finalize()
assert(http.readXml().customer.name == "Joe Green")
http.close()
