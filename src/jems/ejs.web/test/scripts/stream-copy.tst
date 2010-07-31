/*
    Test stream-copy
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.post(HTTP + "/stream-copy.es")
http.write(Path("web/sendfile.txt").readString())
http.finalize()
assert(http.status == 200)
assert(http.response == Path("web/sendfile.txt").readString())
http.close()

