require ejs.web

exports.app = function(r: Request) {
    write("Hello World\r\n")
    finalize()
}
