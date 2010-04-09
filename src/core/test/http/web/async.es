require ejs.web
require ejs.web.jsgi

exports.app = Response(function (request) { 
    return function() {
        setStatus(200)
        setHeaders({"Content-Type": "text/html"})
        Timer(5000, function() {
            request.write("Now done\n")
            request.finish()
        })
    } 
})
