/*
    View.script
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
        case "/script":
            let view = new View(this)
            view.script.apply(view, proxyData)
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
proxy("script", [
    '<script src="/static/js/jquery.js" type="text/javascript"></script>',
    '<script src="/static/js/jquery.tablesorter.js" type="text/javascript"></script>',
    '<script src="/static/js/jquery.address.js" type="text/javascript"></script>',
    '<script src="/static/js/jquery.simplemodal.js" type="text/javascript"></script>',
    '<script src="/static/js/jquery.ejs.js" type="text/javascript"></script>',
])


//  Custom
proxy("script", ["one.js", "two.js"], [
    '<script src="one.js" type="text/javascript"></script>',
    '<script src="two.js" type="text/javascript"></script>',
])

server.close()
