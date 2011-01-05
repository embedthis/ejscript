require ejs.web

exports.app = function() { 
    setStatus(200)
    setHeaders({"Content-Type": "text/plain"})
    dontAutoFinalize()
    on("readable", function(request) {
        let data = new ByteArray
        if (read(data)) {
            write(data)
        } else {
            finalize()
        }
    })
}
