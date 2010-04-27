require ejs.web
require ejs.web.jsgi

exports.app = function (r: Request) { 

    r.setStatus(200)
    r.setHeaders({"Content-Type": "text/plain"})
    let input = r.input
    let data = new ByteArray
    while (input.read(data)) {
        r.write(data)
    }
    r.finalize()
}
