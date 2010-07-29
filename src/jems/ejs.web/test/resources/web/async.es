/*
    Non-blocking async request. Pauses for 5 seconds before completing
 */

require ejs.web

exports.app = function (request: Request) { 
    return function(request: Request) {
        request.setStatus(200)
        request.setHeaders({"Content-Type": "text/html"})
        Timer(5000, function() {
            request.write("Now done\n")
            request.finalize()
        })
    } 
}
