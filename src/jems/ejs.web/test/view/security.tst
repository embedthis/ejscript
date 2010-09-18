/*
    View.securityToken
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
        case "/security":
            let view = new View(this)
            view.securityToken.apply(view, proxyData)
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
proxy("security", [
    '<meta name="SecurityTokenName" content="__ejs_security_token__" />',
    '<meta name="__ejs_security_token__" content="',
])


server.close()
