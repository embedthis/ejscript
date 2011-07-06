/*
    View.list
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
        case "/list":
            let view = new View(this)
            view.form(record)
print("DATA " + proxyData)
print("LENGTH " + proxyData.length)
print("0 " + proxyData[0])
print("1 " + proxyData[1])
            view.list.apply(view, proxyData)
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

//  So we can test the various kinds of list parameters, we store text in the record. 
public var record = {id: 7, name: "Joe", priority: "med", member: true, notes: "Nested <b>html</b>", password: "gold"}

/*
//  List of values
proxy("list", "priority", ["low", "med", "high"], [
    '<select name="priority" >',
        '<option value="0">low</option>',
        '<option value="1" selected="yes">med</option>',
        '<option value="2">high</option>',
    '</select>',
])


//  Pairs for values
proxy("list", "priority", [["low", 0], ["med", 1], ["high", 2]], [
    '<select name="priority" >',
        '<option value="0">low</option>',
        '<option value="1" selected="yes">med</option>',
        '<option value="2">high</option>',
    '</select>',
])


//  Single object hash
proxy("list", "priority", {low: 0, med: 1, high: 2}, [
    '<select name="priority" >',
        '<option value="0">low</option>',
        '<option value="1" selected="yes">med</option>',
        '<option value="2">high</option>',
    '</select>',
])


//  Array of object key/values
proxy("list", "priority", [{"low": 0}, {"med": 1}, {"high": 2}], [
    '<select name="priority" >',
        '<option value="0">low</option>',
        '<option value="1">med</option>',
        '<option value="2">high</option>',
    '</select>'
])
*/


server.close()
