require ejs.web

exports.app = function(request: Request) {
    request.write("Hello World\r\n")
}
