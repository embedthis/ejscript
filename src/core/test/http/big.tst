/*
    Big file transfers
 */

const HTTP = (global.session && session["http"]) || ":6700"

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
