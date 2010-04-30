require ejs.web
require ejs.web.jsgi

exports.app = function (request: Request) { 
    return function(r: Request) {
        r.setStatus(200)
        r.setHeaders({"Content-Type": "text/html"})
        Timer(5000, function() {
            r.write("Now done\n")
            r.finalize()
        })
    } 
}
