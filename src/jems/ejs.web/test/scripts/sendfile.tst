/*
    Test sendfile-*
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http
http.get(HTTP + "/sendfile.es")
assert(http.status == 200)
assert(http.response == Path("web/sendfile.txt").readString())
http.close()

/*
http.get(HTTP + "/sendfile-header.es")
assert(http.status == 200)
assert(http.response == Path("web/sendfile.txt").readString())
http.close()

http.get(HTTP + "/sendfile-path.es")
assert(http.status == 200)
assert(http.response == Path("web/sendfile.txt").readString())
http.close()
*/
