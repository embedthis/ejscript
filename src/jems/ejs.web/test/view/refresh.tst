/*
    Refresh option
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
        switch (pathInfo) {
        case "/label":
            let view = new View(this)
            view.label.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


//  Refresh options
proxy("label", "Text", {refresh: "@update"}, [
    '<span id="id',
    ' data-refresh="/update">Text</span>',
])

//  With period
proxy("label", "Text", {refresh: "@update", period: 2000}, [
    '<span data-refresh-period="2000" id="id',
    ' data-refresh="/update">Text</span>',
])

server.close()
