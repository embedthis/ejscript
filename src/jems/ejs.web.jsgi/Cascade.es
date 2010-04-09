/*
    Cascade slices and return the response from the first non-404 slice
  
    Usage:
        require ejs.web.jsgi
        exports.app = Cascade(app)
 */

module ejs.web.jsgi {

    /** @hide */
    function Cascade(apps, status = 404) {
        return function(request) {
            for each (app in apps) {
                let response = app(request)
                if (response.status !== status) {
                    return response
                }
            }
            return null
        }
    }
}
