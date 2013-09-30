/*
    dynamic-big.tst -- Test dynamic output from a template -- should require transfer chunk encoding
 */

const HTTP = App.config.uris.http

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/dynamic-big.ejs")

assert(http.status == 200)

assert(http.header("date").contains("GMT"))
assert(http.header("Content-Length") == null)
assert(http.header("Keep-Alive"))
assert(http.header("Transfer-Encoding"))
assert(http.header("connection") == "Keep-Alive")
assert(http.response)
http.close()
