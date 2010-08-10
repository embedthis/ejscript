/*
    Test bigger dynamic output
 */
const HTTP = ":" + (App.config.test.http_port || "6700")

const BIG = HTTP + "/big.ejs"
var http: Http = new Http

http.get(BIG)
buf = new ByteArray
while (http.read(buf) > 0) {
    assert(buf.available > 0)
}
assert(http.status == 200)
http.close()

/*  MOB - a bit slow
http.get(BIG)
ts = new TextStream(http)
lines = ts.readLines()
assert(lines.length == 801)
http.close()
*/
