/*
    General attribute options
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
            let view = View(this)
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

proxy("label", "Text", {color: "#123", background: "#FFF"}, '<span color="#123" background="#FFF">Text</span>')
proxy("label", "Text", {height: 23, width: "200px"}, '<span height="23" width="200px">Text</span>')
proxy("label", "Text", {style: "emphatic"}, '<span class="emphatic">Text</span>')
proxy("label", "Text", {effects: "fadeIn"}, '<span data-effects="fadeIn">Text</span>')
proxy("label", "Text", {modal: true}, '<span data-modal="true">Text</span>')
proxy("label", "Text", {"data-custom": "something"}, '<span data-custom="something">Text</span>')
proxy("label", "Text", {"domid": "my-123"}, '<span id="my-123">Text</span>')

server.close()
