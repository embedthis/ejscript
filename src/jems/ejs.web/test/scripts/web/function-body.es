require ejs.web

exports.app = function (request) {
    return function (request) {
        status = 200
        setHeaders({"Content-Type": "text/plain"})
        write("Hello World - Function\n")
    } 
}
