/*
    Test CommonLog wrapper
 */

const HTTP = ":" + (App.config.test.http_port || "6700")
const LOG = Path("access.log")

var http: Http = new Http

var priorLength = LOG.size
http.get(HTTP + "/dispatch.es?route=log")
assert(http.contentLength > 0)
assert(http.status == Http.Ok)
assert(LOG.size > priorLength)
http.close()

