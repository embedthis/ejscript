require ejs.web

exports.app = function (request) { 
    return {
        body: File("web/file-body.txt", "r"),
    } 
}
