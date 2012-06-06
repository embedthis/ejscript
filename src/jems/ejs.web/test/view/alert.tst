/*
    alert()
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
        case "/alert":
            let view = new View(this)
            view.alert.apply(view, proxyData)
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

proxy("alert", "Text", {refresh: "@updates", period: 0}, 
    '<div data-refresh-period="0" class="-ejs-alert" id="id_0" data-refresh="/updates">Text</div>')

server.close()
