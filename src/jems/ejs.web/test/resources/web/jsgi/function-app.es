require ejs.web

exports.app = function (request) {
    return function (request) {
        //  MOB -- request is probably not required - check */
        request.status = 200
        request.setHeaders({"Content-Type": "text/plain"})
        request.write("Hello Beautiful World\n")
        request.finalize()
    } 
}
