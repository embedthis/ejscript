/*
    Put tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http = new Http

//  Test put with data
let path = Path("file.dat")
data = path.readString()
print("MOB put")
http.put(HTTP + "/file.dat", data)
http.finalize()
print("MOB after finalize")
assert(http.status == 201 || http.status == 204)
if (Config.OS != "WIN") {
    assert(path.size == Path("web/file.dat").size)
}
try {Path("web/file.dat").remove()} catch {}
http.close()
print("MOB done 1")


/*
//  Test put with a stream of data
let path = Path("file.dat")
http.setHeader("Content-Length", path.size)
print("MOB put 2")
http.put(HTTP + "/file.dat")
let file = File(path, "r")
buf = new ByteArray
while (file.read(buf)) {
    http.write(buf)
}
print("MOB finalized")
http.finalize()
print("MOB - about to get status")
assert(http.status == 201 || http.status == 204)
http.close()
if (Config.OS != "WIN") {
    assert(path.size == Path("web/file.dat").size)
}
assert(path.size == Path("web/file.dat").size)
try {Path("web/file.dat").remove()} catch {}
*/
