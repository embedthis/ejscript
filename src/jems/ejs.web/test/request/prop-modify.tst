/*
    Test modifying properties inside a web server
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {

    assert(absHome == HTTP + "/")
    absHome = "http://localhost:1234/"
    assert(absHome == "http://localhost:1234/")

    assert(autoFinalizing == true)
    autoFinalizing = false
    assert(!autoFinalizing)

    assert(headers && header("Host") && header("Date") && header("Connection") && header("User-Agent"))
    headers["Custom-Header"] = 1234
    assert(headers["Custom-Header"] == 1234)

    assert(home == "../")
    home = ".."
    assert(home == "..")

    let old = method
    assert(method == "GET")
    method = "POST"
    assert(method == "POST")
    assert(originalMethod == old)

    assert(host == Uri(HTTP).host)
    host = "127.1.2.3"
    assert(host == "127.1.2.3")

    assert(uri == "http://127.1.2.3:6700/index.html")
    host = "localhost"
    uri = "http://localhost:9000/test.html"
    assert(uri == "http://localhost:9000/test.html")

    assert(originalUri == (HTTP + "/index.html"))

    assert(pathInfo == "/index.html")
    pathInfo = "/list/"
    assert(pathInfo == "/list/")
    assert(uri == "http://localhost:6700/list/")
    pathInfo = "/index.html"
    assert(uri == "http://localhost:6700/index.html")

    assert(port == 6700)
    port = 7700
    assert(port == 7700)
    assert(uri == "http://localhost:7700/index.html")
    port = 6700

    assert(query == null)
    assert(reference == null)
    assert(referrer == null)
    assert(remoteAddress == "127.0.0.1")

    assert(responseHeaders)
    responseHeaders["New-Header"] = "abc"
    assert(responseHeaders["New-Header"] == "abc")

    assert(scriptName == "")
    scriptName = "/myApp"
    assert(uri == "http://localhost:6700/myApp/index.html")
    assert(originalUri == (HTTP + "/index.html"))
    assert(pathInfo == "/index.html")
    scriptName = ""
    assert(uri == "http://localhost:6700/index.html")

    pathInfo = "/demo/users/index.html"
    setLocation("/demo/", "/home/demo")
    assert(scriptName == "/demo")
    assert(pathInfo == "/users/index.html")
    assert(dir = "/home/demo")

    //  cleanup
    pathInfo = "/index.html"
    scriptName = ""
    assert(uri == "http://localhost:6700/index.html")

    assert(server && server.documents == "." && server.home == ".")
    assert(status == 200)

    finalize()
})

let http = new Http
http.get(HTTP + "/index.html")
http.wait()
http.close()
server.close()
