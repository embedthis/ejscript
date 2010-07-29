/*
    redirect.tst -- Test redirection of directory requests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.followRedirects = false
http.get(HTTP + "/dir")
assert(http.status == Http.MovedPermanently)
http.close()
