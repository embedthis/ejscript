require ejs.web

exports.app = function() {
    setHeader("X-Sendfile", "web/sendfile.txt")
}
