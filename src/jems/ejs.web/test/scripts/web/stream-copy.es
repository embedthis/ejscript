require ejs.web
require ejs.web.jsgi

exports.app = function() { 
    setStatus(200)
    setHeaders({"Content-Type": "text/plain"})
    observe("readable", function(request) {
        let data = new ByteArray
        if (read(data)) {
            write(data)
        } else {
            finalize()
        }
    })
}
