require ejs.web
require ejs.web.jsgi

/*OK
exports.app = function (request) { 
    return {
        status: 201, 
        headers: {"Content-Type": "text/html"}, 
        body: "Hello Cruel World\n"
    } 
}
*/

/*OK
exports.app = function (request) { 
    return {
        body: ["Hello Cruel World\n"],
    } 
}
*/

/*OK
exports.app = function (request) { 
    return {
        body: ["Hello", " Cruel", " World\n"]
    } 
}
*/


/*OK
exports.app = function (request) { 
    return {
        body: File("web/big.txt").open("r"),
    } 
}
*/


/*OK
exports.app = function (request) {
    return {
        status: 201, 
        headers: {"Content-Type": "text/plain"}, 
        body: "Hello Beautiful World\n"
    } 
}
*/

/*OK
exports.app = Head(function (request) {
        return {
            status: 201, 
            headers: {"Content-Type": "text/html"}, 
            body: "Hello Beautiful World\n"
        } 
    })
*/

/*OK
require ejs.cjs
exports.app = require("web/custom.js").CustomHeader(
    function (request) {
        return {
            status: 201, 
            headers: {"Content-Type": "text/html"}, 
            body: "Hello Cruel World\n"
        } 
    })

*/
