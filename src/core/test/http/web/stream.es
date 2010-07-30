require ejs.web

exports.app = function (request: Request) { 
    request.setStatus(200)
    request.setHeaders({"Content-Type": "text/plain"})
    let input = request.input
    let data = new ByteArray
    while (input.read(data)) {
        request.write(data)
    }
    request.finalize()
}
