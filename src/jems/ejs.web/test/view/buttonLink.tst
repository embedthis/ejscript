/*
    View.buttonLink
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)
load("proxy.es")

server.on("readable", function (event, request: Request) {
    try {
        router.route(request)
        switch (pathInfo) {
        case "/buttonLink":
            let view = new View(this)
            view.buttonLink.apply(view, proxyData)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})


//  Simple buttonLink (use as part of a form)

proxy("buttonLink", "Click Me", '<button>Click Me</button>')

//  Click action 
proxy("buttonLink", "Click Me", "@buy", '<button data-click="/buy">Click Me</button>')

//  Remote click
proxy("buttonLink", "Click Me", {remote: "@buy", apply: "div.cart"}, 
    '<button data-apply="div.cart" data-remote="/buy">Click Me</button>')

server.close()
