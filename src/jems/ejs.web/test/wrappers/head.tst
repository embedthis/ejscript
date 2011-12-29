/*
    Test Head wrapper
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.head(HTTP + "/dispatch.es?route=head")
assert(http.status == 200)
assert(http.response == "")
assert(http.contentLength > 0)

http.close()

