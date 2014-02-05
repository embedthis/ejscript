/*
    Test ContentType wrapper
 */

const HTTP = App.config.uris.http

var http: Http = new Http

http.get(HTTP + "/dispatch.es?route=content")
assert(http.header("Content-Type", "text/x-markdown"))
assert(http.response == "Hello Content")
http.close()
