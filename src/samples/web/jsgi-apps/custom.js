/*
    Custom Header - JSGI middleware to add a custom date header
 */
exports.CustomHeader = function(app) {
    return function(request) {
        var response = app(request)
        response.headers ||= {}
        response.headers["Custom-Header"] = Date().toString()
        return response
    }
}
