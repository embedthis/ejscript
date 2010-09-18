/*
    View.stylesheet
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)

load("proxy.es")

public var proxyData

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        switch (pathInfo) {
        case "/stylesheet":
            let view = new View(this)
            view.stylesheet.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


//  Default
proxy("stylesheet", [
    '<link rel="stylesheet" type="text/css" href="/static/layout.css" />',
    '<link rel="stylesheet" type="text/css" href="/static/themes/default.css" />',
])


//  Custom
proxy("stylesheet", ["one.css", "two.css"], [
    '<link rel="stylesheet" type="text/css" href="one.css" />',
    '<link rel="stylesheet" type="text/css" href="two.css" />',
])

server.close()
