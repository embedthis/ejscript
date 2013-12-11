/*
    redirect.tst -- Test redirection of directory requests
 */

const HTTP = App.config.uris.http

var http: Http = new Http

http.followRedirects = false
http.get(HTTP + "/dir")
assert(http.status == Http.MovedPermanently)
http.close()
