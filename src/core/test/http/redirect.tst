/*
    Redirect tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

http.get(HTTP + "/dir")
http.followRedirects = false
assert(http.status == 301)

http.followRedirects = true
http.get(HTTP + "/dir")
assert(http.status == 200)
http.close()
