require ejs.web

exports.app = function() {
    return {
        body: Path("web/sendfile.txt")
    }
}
