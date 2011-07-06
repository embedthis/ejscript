require ejs.web

exports.app = function() { 
    setHeader("Content-Type", "text/plain")
    dontAutoFinalize()

    let sofar = 0
    on("readable", function(request) {
        let data = new ByteArray
        while (rc = read(data)) {
            write(data)
        }
        if (rc == null) {
            finalize()
        }
    })
}
