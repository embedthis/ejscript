/*
    index.tst. Test auto adding of indexes without redirects.
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Should not need to follow redirects. Adding index should be transparent
http.followRedirects = false
http.get(HTTP + "/dir/")
assert(http.status == 200)
http.close()
