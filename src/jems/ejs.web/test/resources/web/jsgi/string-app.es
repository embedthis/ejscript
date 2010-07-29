
exports.app = function(request) { 
    return {
        status: 200, 
        headers: {"Content-Type": "text/plain"}, 
        body: "Hello World - String\n"
    } 
}
