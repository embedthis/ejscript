require ejs.web

exports.app = function() {
    sendfile("web/big.txt")
}
