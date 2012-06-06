/*
    Reset tests
 */

const HTTP = App.config.uris.http
var http: Http

http = new Http
http.setHeader("Content-Length", 1000)
http.post(HTTP + "/index.html")
http.reset()
http.setHeader("Content-Length", 1000)
http.post(HTTP + "/index.html")
http.reset()
http.get(HTTP + "/index.html")
assert(http.status == 200)
http.close()
