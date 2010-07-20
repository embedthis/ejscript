/*
    Redirect tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http

/*
http.get(HTTP + "/dir")
http.followRedirects = false
assert(http.status == 301)
http.followRedirects = true
http.get(HTTP + "/dir")
assert(http.status == 200)
http.close()
*/
