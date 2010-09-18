/*
    View.image
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
        case "/image":
            let view = new View(this)
            view.image.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


//  Basic
proxy("image", "weather.png", '<img src="weather.png"/>')


//  Clickable
proxy("image", "weather.png", "@expand", '<img src="weather.png" data-click="/expand"/>')


//  Refresh
proxy("image", "weather.png", {refresh: "@expand", period: 2000}, 
    '<img src="weather.png" data-refresh-period="2000" id="id_0" data-refresh="/expand"/>')


server.close()
