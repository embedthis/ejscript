/*
    Test bigger dynamic output
 */
const HTTP = App.config.uris.http

const BIG = HTTP + "/big.es"
var http: Http = new Http

http.get(BIG)
buf = new ByteArray
while (http.read(buf) > 0) {
    assert(buf.available > 0)
}
assert(http.status == 200)
http.close()
