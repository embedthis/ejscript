/*
    dynamic-small.tst -- Test a form and post-back
 */

const HTTP = App.config.uris.http

var http: Http = new Http

//  Test a basic get first
http.get(HTTP + "/dynamic-small.ejs")
assert(http.status == 200)
assert(http.header("Server").contains("ejs-http/"))
assert(http.header("Date").contains("GMT"))
assert(http.header("Content-Length") == 1976)
assert(http.header("Keep-Alive"))
assert(!http.header("Transfer-Encoding"))

//  Should not be a transfer-encoding header
assert(http.header("transfer-encoding") == null)
assert(http.header("connection") == "keep-alive")
assert(http.response)
http.close()
