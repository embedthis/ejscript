/*
    Custom Header
 */
exports.CustomHeader = function(app) {
    return function(request) {
        var response = app(request)
        response.headers ||= {}
        response.headers["Custom-Header"] = Date().toString()
        return response
    }
}
