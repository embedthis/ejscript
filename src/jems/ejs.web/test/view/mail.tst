/*
    View.mail
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
        case "/mail":
            let view = new View(this)
            view.mail.apply(view, proxyData)
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
proxy("mail", "Mary Poppins", "mary.poppins@example.com", '<a href="mailto:mary.poppins@example.com">Mary Poppins</a>')

server.close()
