
exports.app = function (request) {
    return function (request) {
        request.status = 200
        request.setHeaders({"Content-Type": "text/plain"})
        request.write("Hello Beautiful World\n")
        request.finalize()
    } 
}
