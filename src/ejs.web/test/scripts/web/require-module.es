require ejs.web

exports.app = require("web/custom.js").CustomHeader(
    function (request) {
        return {
            status: 200, 
            headers: {"Content-Type": "text/plain"}, 
            body: "Hello World - Custom Header\n"
        } 
    }
)

