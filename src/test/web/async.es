require ejs.web

exports.app = function (r: Request) { 
    setStatus(200)
    setHeaders({"Content-Type": "text/html"})
    Timer(5000, function() {
        write("Now done\n")
        finalize()
    }).start()
}
