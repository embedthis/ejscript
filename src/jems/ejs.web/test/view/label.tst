/*
    View.label
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

//  Bare without options
proxy("label", "Text", '<span>Text</span>')

//  Clickable
proxy("label", "Text", "login", '<span data-click="login">Text</span>') 
proxy("label", "Text", "@login", '<span data-click="/login">Text</span>') 
proxy("label", "Text", "@Admin/", '<span data-click="/Admin">Text</span>') 
proxy("label", "Text", "@Admin/login", '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", "/Admin/login", '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", "http://example.com/Admin/login", '<span data-click="http://example.com/Admin/login">Text</span>') 

server.close()
