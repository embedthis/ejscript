require ejs.web
require ejs.web.jsgi

exports.app = function (request) { 
    return function(r) {
        r.setStatus(200)
        r.setHeaders({"Content-Type": "text/html"})
        Timer(5000, function() {
            r.write("Now done\n")
            r.finish()
            r.finalize()
        })
    } 
}
