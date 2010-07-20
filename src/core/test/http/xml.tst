/*
    XML tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http
http.get(HTTP + "/test.xml")
assert(http.readXml().customer.name == "Joe Green")
http.close()
