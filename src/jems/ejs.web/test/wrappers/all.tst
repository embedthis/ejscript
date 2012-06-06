/*
    Test all wrappers
 */

const HTTP = App.config.uris.http

var http: Http = new Http
http.head(HTTP + "/dispatch.es?route=all")
assert(http.response == "")
assert(http.contentLength > 0)
assert(http.status == Http.Ok)
http.close()

