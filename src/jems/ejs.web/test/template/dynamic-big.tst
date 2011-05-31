/*
    dynamic-big.tst -- Test dynamic output from a template -- should require transfer chunk encoding
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/dynamic-big.ejs")

assert(http.status == 200)
assert(http.header("Server").contains("ejs-http/"))
assert(http.header("date").contains("GMT"))
assert(http.header("Content-Length") == null)
assert(http.header("Keep-Alive"))
assert(http.header("Transfer-Encoding"))

//  Should not be a transfer-encoding header
assert(http.headers["transfer-encoding"])
assert(http.headers.connection == "keep-alive")
assert(http.response)
http.close()
