require ejs.web

exports.app = Head(function (request) {
    return {
        status: 200, 
        headers: {"Content-Type": "text/plain"}, 
        body: "Hello World - Head (not transmitted)\n"
    } 
})

