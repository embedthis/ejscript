/*
    View.div()
 */
require ejs.web

const HTTP = App.config.uris.http

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)
load("proxy.es")

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        switch (pathInfo) {
        case "/div":
            let view = new View(this)
            view.div.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


proxy("div", "<p>nested html</p>", '<div><p>nested html</p></div>')

//  clickable
proxy("div", "<p>nested html</p>", { click: "@bing" }, '<div data-click="/bing"><p>nested html</p></div>')

server.close()
