/*
    Lowest level raw I/O directly to the request object
 */
require ejs.web

exports.app = function() {
    write("Hello World\r\n")
    finalize()
}
