/*
    View.input
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
        case "/input":
            let view = new View(this)
            view.form(record)
            view.input.apply(view, proxyData)
            view.endform()
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
    } catch (e) {
        writeError(Http.ServerError, e)
    }
})

public var record = {id: 7, name: "Joe", height: 6.0, member: true, notes: "Nested <b>html</b>", password: "gold"}

//  Plain input field from record
proxy("input", "name", [
    '<input name="id" type="hidden" value="7" />',
    '<input name="name" type="text" value="Joe" />'
])

proxy("input", "height", ['<input name="height" type="text" value="6" />'])

//  Boolean type
proxy("input", "member", ['<input name="member" type="checkbox" checked="yes" value="true" />'])

server.close()
