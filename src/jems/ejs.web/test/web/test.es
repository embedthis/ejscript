/*
    Simple JSGI style app
 */
exports.app = function (request) { 
    return {
        status: 200, 
        headers: {"Content-Type": "text/html"}, 
        body: "Hello Cruel World\n"
    } 
}
