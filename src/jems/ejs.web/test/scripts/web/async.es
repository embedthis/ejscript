/*
    Non-blocking async request. Pauses for 5 seconds before completing
 */

require ejs.web

exports.app = function (request: Request) { 
    return function(request: Request) {
        setStatus(200)
        setHeaders({"Content-Type": "text/html"})
        Timer(50, function() {
            request.write("Now done\n")
            request.finalize()
        }).start()
    } 
}
