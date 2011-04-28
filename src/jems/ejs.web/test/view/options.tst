/*
    General View control options
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

//  Bare without options
proxy("label", "Text", '<span>Text</span>')

//  Empty options
proxy("label", "Text", {}, '<span>Text</span>')

//  Options string
proxy("label", "Text", "login", '<span data-click="login">Text</span>') 
proxy("label", "Text", "@login", '<span data-click="/login">Text</span>') 
proxy("label", "Text", "@Admin/", '<span data-click="/Admin">Text</span>') 
proxy("label", "Text", "@Admin/login", '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", "/Admin/login", '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", "http://example.com/Admin/login", '<span data-click="http://example.com/Admin/login">Text</span>') 

//  Options for click
proxy("label", "Text", {click: "@login"}, '<span data-click="/login">Text</span>') 
proxy("label", "Text", {click: "@Admin/login"}, '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", {click: "@login", controller: "User"}, '<span data-click="/User/login">Text</span>') 
proxy("label", "Text", {click: "http://example.com/test.php"}, '<span data-click="http://example.com/test.php">Text</span>') 

proxy("label", "Text", {click: {uri: "http://example.com/test.php"}}, '<span data-click="http://example.com/test.php">Text</span>') 
proxy("label", "Text", {click: {scheme: "http", host: "example.com", path: "test.php"}}, 
    '<span data-click="http://example.com/test.php">Text</span>') 
proxy("label", "Text", {click: {controller: "Admin", action: "login"}}, '<span data-click="/Admin/login">Text</span>') 
proxy("label", "Text", {click: {controller: "Admin", action: "login", method: "PUT"}}, 
    '<span data-click="/Admin/login" data-click-method="PUT">Text</span>')
proxy("label", "Text", {abc: 77, def: "blue"}, '<span>Text</span>') 

//  Remote options
proxy("label", "Text", {remote: "@login"}, '<span data-remote="/login">Text</span>') 
proxy("label", "Text", {remote: {controller: "Admin", action: "login", method: "PUT"}}, 
    '<span data-remote="/Admin/login" data-remote-method="PUT">Text</span>')

//  Refresh options
proxy("label", "Text", {refresh: "@update"}, '<span id="id_0" data-refresh="/update">Text</span>')
proxy("label", "Text", {refresh: {controller: "Dash", action: "update", method: "HEAD"}}, 
    '<span id="id_0" data-refresh="/Dash/update" data-refresh-method="HEAD">Text</span>')

//  Combined click and refresh
proxy("label", "Text", {click: "select", refresh: "@update", period: 20}, 
    '<span data-refresh-period="20" data-click="select" id="id_0" data-refresh="/update">Text</span>')

//  Escaped text
proxy("label", "<Text>", {escape: true}, '<span>&lt;Text&gt;</span>')

//  Set DOM ID
proxy("label", "Text>", {domid: 999}, '<span id="999">Text></span>') 

//  Params
proxy("label", "Text", {click: "/login", params: {name: "Bob", weight: 175.5}, method: "POST"}, 
    '<span data-click="/login" data-click-method="POST" data-click-params="name=Bob&weight=175.5">Text</span>')

//  Query
proxy("label", "Text", {click: { path: "/test.php", query: "a=b&c=d"}}, '<span data-click="/test.php?a=b&c=d">Text</span>')

server.close()
