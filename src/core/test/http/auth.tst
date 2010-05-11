/*
    Authentication tests
 */

const HTTP = (global.session && session["http"]) || ":6700"

test.skip("auth")

/* MOB
var http: Http = new Http

http.get(HTTP + "/basic/basic.html")
assert(http.status == 401)

http = new Http
http.setCredentials("joshua", "pass1")
http.get(HTTP + "/basic/basic.html")
assert(http.status == 200)
http.close()
*/
