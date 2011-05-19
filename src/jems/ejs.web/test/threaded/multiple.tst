/*
    multiple.tst -- Multiple overlapped requests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var nap: Http = new Http

//  Issue a nap request. This request takes 2 seconds to complete -- waited on below.
nap.get(HTTP + "/nap.ejs")


//  Overlapped non-blocking request
var http: Http = new Http
for (i in 20) {
    http.get(HTTP + "/index.ejs")
    assert(http.status == 200)
}
http.close()


//  Wait for nap request
assert(nap.status == 200)
nap.close()
