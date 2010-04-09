/*
    Put tests
 */

const HTTP = (global.session && session["http"]) || ":6700"
var http: Http = new Http

data = Path("file.dat").readString()
http.put(HTTP + "/tmp/file.dat", data)
assert(http.status == 201 || http.status == 204)

http.bodyLength = Path("file.dat").size
http.put(HTTP + "/tmp/file.dat")
file = Path("file.dat").open()
buf = new ByteArray
while (file.read(buf) > 0) {
    http.write(buf)
}
http.finalize()
assert(http.status == 201 || http.status == 204)
http.close()
