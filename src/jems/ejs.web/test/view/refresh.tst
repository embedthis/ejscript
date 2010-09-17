/*
    Refresh option
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)
load("proxy.es")

public var labelData = {}

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        switch (pathInfo) {
        case "/label":
            View(this).label(labelData.text, labelData.options)
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
proxy("label", "Text", {refresh: "@update"}, '<span id="id_0" data-refresh="/update">Text</span>')

//  With period
proxy("label", "Text", {refresh: "@update", period: 2000}, 
    '<span data-refresh-period="2000" id="id_0" data-refresh="/update">Text</span>')

server.close()
