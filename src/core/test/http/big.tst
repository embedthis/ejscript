/*
    Big file transfers
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

const BIG = HTTP + "/big.ejs"
var http: Http = new Http

http.get(BIG)
buf = new ByteArray
while (http.read(buf) > 0) {
    assert(buf.available > 0)
}
http.close()

http.get(BIG)
ts = new TextStream(http)
lines = ts.readLines()
assert(lines.length == 801)
http.close()
