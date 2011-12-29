/*
    View.flash
 */
require ejs.web

const HTTP = App.config.uris.http

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)

load("proxy.es")

public var proxyData

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        setupFlash()
        switch (pathInfo) {
        case "/flash":
            let view = new View(this)
            view.flash.apply(view, proxyData)
            request.finalizeFlash()
            close()
            break

        case "/notify":
            request.notify.apply(request, proxyData)
            request.finalizeFlash()
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


//  Set a warning and it should persist for only one request
proxy("notify", "warn", "Storm coming", null)
proxy("flash", '<div class="-ejs-flash -ejs-flash-warn">Storm coming</div>')
proxy("flash", '')

proxy("notify", "inform", "Good Sailing", null)
proxy("flash", '<div class="-ejs-flash -ejs-flash-inform">Good Sailing</div>')
proxy("flash", '')

server.close()
