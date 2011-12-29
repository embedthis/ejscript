/*
    Remote options
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

//  Remote options
proxy("label", "Text", {remote: "@login"}, '<span data-remote="/login">Text</span>') 

proxy("label", "Text", {remote: "@login", apply: "div.content"}, 
    '<span data-apply="div.content" data-remote="/login">Text</span>')

proxy("label", "Text", {remote: {controller: "Admin", action: "login", method: "PUT"}}, 
    '<span data-remote="/Admin/login" data-remote-method="PUT">Text</span>')

server.close()
