
exports.app = function (request) { 
    return {
        body: File("data.txt").open("r"),
    } 
}
