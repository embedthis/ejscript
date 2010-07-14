require ejs.web
require ejs.web.jsgi

exports.app = function (request: Request) { 

print("IN FUNCTION")
    request.setStatus(200)
    request.setHeaders({"Content-Type": "text/plain"})
    let input = request.input
    let data = new ByteArray
print("ABOUT TO READ")
    while (input.read(data)) {
print("LEN " + data.available)
        request.write(data)
    }
print("DONE")
    request.finalize()
}
