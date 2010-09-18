/*
    View.tabs
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
        case "/tabs":
            let view = new View(this)
            view.form(record)
            view.tabs.apply(view, proxyData)
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


//  Simple clicks
proxy("tabs", {File: "file.html", Edit: "edit.html", View: "view.html"}, [
    '<div class="-ejs-tabs">',
        '<ul>',
        '<li data-click="/file.html">File</li>',
        '<li data-click="/edit.html">Edit</li>',
        '<li data-click="/view.html">View</li>',
        '</ul>',
    '</div>',
])


//  Array of object key/values
proxy("tabs", [{File: "div.file"}, {Edit: "div.edit"}, {View: "div.view"}], {toggle: true}, [
    '<div class="-ejs-tabs">',
        '<ul>',
        '<li data-toggle="div.file">File</li>',
        '<li data-toggle="div.edit">Edit</li>',
        '<li data-toggle="div.view">View</li>',
        '</ul>',
    '</div>',
])


//  Show / hide
proxy("tabs", {File: "div.file", Edit: "div.edit", View: "div.view"}, {toggle: true}, [
    '<div class="-ejs-tabs">',
        '<ul>',
        '<li data-toggle="div.file">File</li>',
        '<li data-toggle="div.edit">Edit</li>',
        '<li data-toggle="div.view">View</li>',
        '</ul>',
    '</div>',
])


//  Remote
proxy("tabs", {File: "@Display/file", Edit: "@Display/edit", View: "@Display/view"}, {remote: true}, [
    '<div class="-ejs-tabs">',
        '<ul>',
        '<li data-remote="/Display/file">File</li>',
        '<li data-remote="/Display/edit">Edit</li>',
        '<li data-remote="/Display/view">View</li>',
        '</ul>',
    '</div>',
])


server.close()
