/*
    View.checkbox
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
        case "/checkbox":
            let view = new View(this)
            view.form(record)
            view.checkbox.apply(view, proxyData)
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


//  Boolean checkbox

proxy("checkbox", "member", ['<input name="member" type="checkbox" checked="yes" value="true" />'])


//  Explicit checked value provided

proxy("checkbox", "height", 6.0, [
    '<input name="height" type="checkbox" checked="yes" value="6" />',
    '<input name="height" type="hidden" value="" />'
])


//  Override value
proxy("checkbox", "height", 7.0, { value: 7 }, [
    '<input name="height" type="checkbox" checked="yes" value="7" />',
    '<input name="height" type="hidden" value="" />'
])

server.close()
