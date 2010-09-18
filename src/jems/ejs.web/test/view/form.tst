/*
    View.form and endform
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
        case "/form":
            let view = new View(this)
            view.form.apply(view, proxyData)
            view.endform()
            close()
            break

        case "/fields":
            let view = new View(this)
            view.form.apply(view, proxyData)
            view.text("name")
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


//  Simple form (use as part of a form)
proxy("form", null, "@register", [
    '<form method="POST" action="/register">',
    '<input name="__ejs_security_token__" type="hidden" value="',
    '</form>',
])


//  With new data
let noID  = {name: "Joe", height: 6.0}
proxy("form", noID, "@register", [
    '<form method="POST" action="/register">',
    '<input name="__ejs_security_token__" type="hidden" value="',
    '</form>',
])


//  With existing data

let withID = {id: 7, name: "Joe", height: 6.0}
proxy("form", withID, "@register", [
    '<form method="POST" action="/register" data-click-method="PUT">',
    '<input name="__ejs_security_token__" type="hidden" value="',
    '</form>',
])


//  Method override (custom method LIST)
let withID = {id: 7, name: "Joe", height: 6.0}
proxy("form", withID, { action: "@register", method: "LIST" }, [
    '<form method="POST" action="/register" data-click-method="LIST">',
    '<input name="__ejs_security_token__" type="hidden" value="',
    '</form>',
])


//  With input text field using supplied data record. Scheme derived from record property names.
let withID = {id: 7, name: "Joe", height: 6.0}
proxy("fields", withID, { action: "@register", method: "LIST" }, [
    '<form method="POST" action="/register" data-click-method="LIST">',
    '<input name="__ejs_security_token__" type="hidden" value="',
    '<input name="name" type="text" value="Joe" />'
    '</form>',
])

server.close()
