/*
    Head tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

var http: Http = new Http

test.skip("head")

/* MOB
http.head(HTTP + "/index.html")
assert(http.status == 200)
assert(http.response == "")
assert(http.contentLength > 0) 
http.close()
*/
