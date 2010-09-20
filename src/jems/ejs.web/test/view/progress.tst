/*
    progress()
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
        case "/progress":
            let view = new View(this)
            view.progress.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})

//  Alert with refresh

proxy("progress", 23.1, [
    '<div class="-ejs-progress">',
        '<div class="-ejs-progress-inner" data-progress="23.1">23.1%</div>',
    '</div>>',
])

server.close()
