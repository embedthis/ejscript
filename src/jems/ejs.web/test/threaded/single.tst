/*
    single.tst -- Single non blocking request
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.get(HTTP + "/index.ejs")
assert(http.status == 200)
http.close()
