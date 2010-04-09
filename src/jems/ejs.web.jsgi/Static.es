
module ejs.web.jsgi {

    /** @hide */
    function Static(app, options = {}) {
        let urls = options.urls || ["/favicon.ico"]
        let root = options.root || request.dir
        return function(request) {
            if (urls.contains(request.pathInfo)) {
                let path = root + request.pathInfo
                return { status: 200, headers: "content-type: application/octet-stream", body: File(path, "r") }
            }
            return app(request);
        }
    }
}
