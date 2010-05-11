/*
    Method tests
 */

const HTTP = (global.session && session["http"]) || ":6700"
var http: Http = new Http

test.skip("method")
/* MOB
http.options(HTTP + "/index.html")
assert(http.header("Allow") == "OPTIONS,GET,HEAD,POST,PUT,DELETE")
http.close()

http.trace(HTTP + "/index.html")
assert(http.status == 406)
http.close()
*/
