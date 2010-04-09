/*
    Head.es - Return only the headers of the request for a HTTP HEAD method.
  
    NOTE: This is typically done by good web servers anyway
  
    Usage:
        require ejs.web.jsgi
        exports.app = Head(app)
 */

module ejs.web.jsgi {
    /** @hide */
    function Head(app) {
        return function(request) {
            var response = app(request)
            if (request.method == "HEAD") {
                response.body = []
            }
            return response
        }
    }
}
