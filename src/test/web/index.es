require ejs.web

exports.app = function(r: Request) {
    //  MOB -- how to get request == this
    r.write("Hello World\r\n")
}
