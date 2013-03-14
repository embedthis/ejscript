/*
    simple.tst -- Test SSL test
 */

const HTTPS = App.config.uris.ssl
var http: Http = new Http

print('GET', HTTPS + "/index.html")
http.verify = false
http.get(HTTPS + "/index.html")

assert(http.status == 200)
assert(http.response == Path("../web/index.html").readString())
http.close()
