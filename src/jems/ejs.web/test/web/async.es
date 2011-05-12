require ejs.web

exports.app = function (r: Request) { 
    setStatus(200)
    setHeaders({"Content-Type": "text/html"})
    dontAutoFinalize()

    Timer(30 * 1000, function() {
        try {
            for each (i in 800) {
                write(" Line: %05d %s".format(i, "aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>\r\n"))
            } 
            finalize()
        } catch {}
    }).start()
}
