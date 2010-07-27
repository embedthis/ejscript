require ejs.web

exports.app = function (r: Request) { 
    setStatus(200)
    setHeaders({"Content-Type": "text/html"})

    // dump(r.server.limits)
    // r.setLimits({ transmission: 1024 })
    // dump(r.limits)

    Timer(1000, function() {
        for each (i in 800) {
            write(" Line: %05d %s".format(i, "aaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccccddddddd<br/>\r\n"))
        } 
        finalize()
    }).start()
}
