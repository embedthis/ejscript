//  MOB -- which of these are required?
require ejs.web
require ejs.web.jsgi
require ejs.cjs

exports.app = require("custom.js").CustomHeader(
    function (request) {
        return {
            status: 200, 
            headers: {"Content-Type": "text/plain"}, 
            body: "Hello World - Custom Header\n"
        } 
    })

*/
