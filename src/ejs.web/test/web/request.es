/* GENERATED */
require ejs.web

exports.app = function (request: Request) {
    View(request).render(function(request: Request) {
        setHeader("Content-Type", "application/json")
        show(request)
    })
}
