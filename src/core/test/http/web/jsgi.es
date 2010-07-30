require ejs.web

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
        body: File("web/big.txt", "r")
    } 
}
*/

/*OK
exports.app = Response(function (request) {
        return {
            status: 201, 
            headers: {"Content-Type": "text/html"}, 
            body: "Hello Beautiful World\n"
        } 
    })
*/

/*OK
exports.app = Cascade(Response(function (request) {
        return {
            status: 201, 
            headers: {"Content-Type": "text/html"}, 
            body: "Hello Beautiful World\n"
        } 
    }))
*/

/*OK
require ejs.cjs
exports.app = require("jsgi-test").AddHeader(
    function (request) {
        return {
            status: 201, 
            headers: {"Content-Type": "text/html"}, 
            body: "Hello Cruel World\n"
        } 
    })
*/
