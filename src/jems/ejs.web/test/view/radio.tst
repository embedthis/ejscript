/*
    View.radio
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
        case "/radio":
            let view = new View(this)
            view.form(record)
            view.radio.apply(view, proxyData)
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

public var record = {id: 7, name: "Joe", priority: "med", member: true, notes: "Nested <b>html</b>", password: "gold"}


//  List of values
proxy("radio", "priority", ["low", "med", "high"], [
    'Low <input type="radio" name="priority" value="low" />',
    'Med <input type="radio" name="priority" value="med" checked />',
    'High <input type="radio" name="priority" value="high" />',
])


//  Pairs for values
proxy("radio", "priority", [["low", 0], ["med", 1], ["high", 2]], [
    'Low <input type="radio" name="priority" value="0" />',
    'Med <input type="radio" name="priority" value="1" />',
    'High <input type="radio" name="priority" value="2" />',
])


//  Single object hash
proxy("radio", "priority", {low: 0, med: 1, high: 2}, [
    'Low <input type="radio" name="priority" value="0" />',
    'Med <input type="radio" name="priority" value="1" />',
    'High <input type="radio" name="priority" value="2" />'
])


//  Array of object key/values
proxy("radio", "priority", [{"low": 0}, {"med": 1}, {"high": 2}], [
    'Low <input type="radio" name="priority" value="0" />',
    'Med <input type="radio" name="priority" value="1" />',
    'High <input type="radio" name="priority" value="2" />',
])


server.close()
