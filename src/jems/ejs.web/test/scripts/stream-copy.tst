/*
    Test stream-copy
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.post(HTTP + "/stream-copy.es")
http.write(Path("web/sendfile.txt").readString())
http.finalize()
assert(http.status == 200)
assert(http.response == Path("web/sendfile.txt").readString())
http.close()

