/*
    Put tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Test put with data
let path = Path("file.dat")
data = path.readString()
http.put(HTTP + "/file.dat", data)
http.finalize()
assert(http.status == 201 || http.status == 204)
assert(path.size == Path("web/file.dat").size)
Path("web/file.dat").remove()
http.close()


//  Test put with a stream of data
let path = Path("file.dat")
http.setHeader("Content-Length", path.size)
http.put(HTTP + "/file.dat")
let file = File(path, "r")
buf = new ByteArray
while (file.read(buf)) {
    http.write(buf)
}
http.finalize()
assert(http.status == 201 || http.status == 204)
http.close()
assert(path.size == Path("web/file.dat").size)
Path("web/file.dat").remove()
