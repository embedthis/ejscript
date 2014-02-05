require ejs.web

exports.app = function(r: Request) {
    setHeader("X-Sendfile", "web/big.txt")
    // write("Hello World\r\n")
    // finalize()
}
