/*
    General View control options
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

router = new Router(Router.Top)
server = new HttpServer
server.listen(HTTP)
load("../utils.es")

let labelData = {}

server.on("readable", function (event, request: Request) {
    //try {
        router.route(request)
        switch (pathInfo) {
        case "/label":
            View(this).label(labelData.text, labelData.options)
            close()
            break

        default:
            writeError(Http.ServerError, "Bad test URI")
        }
/*
    } catch (e) {
        writeError(Http.ServerError, e)
    }
*/
})

/*
    Proxy label function
 */
function label(text, options, expected) {
    labelData = { text: text, options: options }
    let http = fetch(HTTP + "/label")
    let response = http.response.trim()
    try {
        //  Validate generated control response
        if (expected is String) {
            assert(response == expected)
        } else if (expected is Array) {
            for each (part in expected) {
                assert(response.contains(part))
            }
        } else {
            for (let [key, value] in expected) {
                assert(response.contains(value))
            }
        }
    } catch (e) {
        print("Assertion failure:")
        print("Expected: \"" + expected + "\"")
        print("Actual:   \"" + response + "\"")
        print(response)
        print(response.length, expected.length)
        throw e
    }
    http.close()
}

//  Bare without options
label("Text", '<span>Text</span>')

//  Empty options
label("Text", {}, '<span>Text</span>')

//  Options string
label("Text", "login", '<span data-click="login">Text</span>') 
label("Text", "@login", '<span data-click="/login">Text</span>') 
label("Text", "@Admin/", '<span data-click="/Admin">Text</span>') 
label("Text", "@Admin/login", '<span data-click="/Admin/login">Text</span>') 
label("Text", "/Admin/login", '<span data-click="/Admin/login">Text</span>') 
label("Text", "http://example.com/Admin/login", '<span data-click="http://example.com/Admin/login">Text</span>') 

//  Options for click
label("Text", {click: "@login"}, '<span data-click="/login">Text</span>') 
label("Text", {click: "@Admin/login"}, '<span data-click="/Admin/login">Text</span>') 
label("Text", {click: "@login", controller: "User"}, '<span data-click="/User/login">Text</span>') 
label("Text", {click: "http://example.com/test.php"}, '<span data-click="http://example.com/test.php">Text</span>') 

label("Text", {click: {uri: "http://example.com/test.php"}}, '<span data-click="http://example.com/test.php">Text</span>') 
label("Text", {click: {scheme: "http", host: "example.com", path: "test.php"}}, 
    '<span data-click="http://example.com/test.php">Text</span>') 
label("Text", {click: {controller: "Admin", action: "login"}}, '<span data-click="/Admin/login">Text</span>') 
label("Text", {click: {controller: "Admin", action: "login", method: "PUT"}}, 
    '<span data-click="/Admin/login" data-click-method="PUT">Text</span>')
label("Text", {abc: 77, def: "blue"}, '<span>Text</span>') 

//  Remote options
label("Text", {remote: "@login"}, '<span data-remote="/login">Text</span>') 
label("Text", {remote: {controller: "Admin", action: "login", method: "PUT"}}, 
    '<span data-remote="/Admin/login" data-remote-method="PUT">Text</span>')

//  Refresh options
label("Text", {refresh: "@update"}, '<span id="id_0" data-refresh="/update">Text</span>')
label("Text", {refresh: {controller: "Dash", action: "update", method: "HEAD"}}, 
    '<span id="id_0" data-refresh="/Dash/update" data-refresh-method="HEAD">Text</span>')

//  Combined click and refresh
label("Text", {click: "select", refresh: "@update", period: 20}, 
    '<span data-refresh-period="20" data-click="select" id="id_0" data-refresh="/update">Text</span>')

//  Escaped text
label("<Text>", {escape: true}, '<span>&lt;Text&gt;</span>')

//  Set DOM ID
label("Text>", {domid: 999}, '<span id="999">Text></span>') 

server.close()
