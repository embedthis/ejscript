/*
    View.text
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
        case "/text":
            let view = new View(this)
            view.form(record)
            view.text.apply(view, proxyData)
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

public var record = {id: 7, name: "Joe", height: 6.0, notes: "Nested <b>html</b>", password: "gold"}


//  Pain input field from record
proxy("text", "name", ['<input name="name" type="text" value="Joe" />'])


//  Escaped
proxy("text", "notes", {escape: true}, ['<input name="notes" type="text" value="Nested &lt;b&gt;html&lt;/b&gt;" />'])


//  Password
proxy("text", "password", {password: true}, ['<input name="password" type="password" value="gold" />'])


//  Value not in record
proxy("text", "unique", ['<input name="unique" type="text" value="" />'])


//  Override value
proxy("text", "unique", {value: "default-value"}, ['<input name="unique" type="text" value="default-value" />'])


//  Set column size
proxy("text", "name", {size: 20}, ['<input name="name" size="20" type="text" value="Joe" />'])


//  Multi-row input

proxy("text", "name", {rows: 20, size: 30}, [
    '<textarea name="name" type="text" cols="30" rows="20">Joe</textarea>'
])


server.close()
