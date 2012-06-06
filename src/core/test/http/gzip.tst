/*
    Get tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.setHeader("Accept-Encoding", "gzip")
http.get(HTTP + "/compressed.txt")
assert(http.status == 200)
http.close()
