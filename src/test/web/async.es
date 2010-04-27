require ejs.web
require ejs.web.jsgi

//  MOB _- Response does not exist

exports.app = Response(function (request) { 
    return function() {
        //  MOB -- how is setStatus resolved?
        setStatus(200)
        setHeaders({"Content-Type": "text/html"})
        Timer(5000, function() {
            request.write("Now done\n")
            request.finish()
        })
    } 
})
