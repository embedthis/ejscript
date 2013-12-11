require ejs.web
require ejs.web.jsgi

exports.app = function (r: Request) { 
    print("HERE")
    setStatus(200)
    setHeaders({"Content-Type": "text/plain"})
    let data = new ByteArray
    /* Echo input -- not only works for forms with URL encoding because data must be buffered first */
    while (read(data)) {
        print("WRITE " + data)
        write(data)
    }
    finalize()
}
