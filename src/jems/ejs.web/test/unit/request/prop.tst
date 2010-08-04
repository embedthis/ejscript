/*
    Test request properties inside a web server
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)

server.observe("readable", function (event, request: Request) {

    assert(absHome == "http://127.0.0.1" + HTTP + "/")
    assert(authGroup == null)
    assert(authType == null)
    assert(authUser == null)
    assert(autoFinalize == true)
    assert(config && config.log && config.web)
    assert(contentLength == -1);
    assert(contentType == null)
    assert(cookies == null)
    assert(dir == ".")
    assert(encoding == null)
    assert(errorMessage == null)
    assert(extension == "html")
    assert(files == null)
    assert(filename == "index.html")
    assert(headers && headers.host && headers.date && headers.connection && headers["user-agent"])
    assert(home == "../")
    assert(host == "localhost")
    assert(isSecure == false)
    assert(limits)
    assert(localAddress == "127.0.0.1")
    assert(log == App.log)
    assert(method == "GET")
    assert(originalMethod == null)
    assert(originalUri == "http://localhost:6700/index.html")
    assert(params.toJSON() == "{}")
    assert(pathInfo == "/index.html")
    assert(port == 6700)
    assert(protocol == "HTTP/1.1")
    assert(query == null)
    assert(reference == null)
    assert(referrer == null)
    assert(remoteAddress == "127.0.0.1")
    assert(responseHeaders && responseHeaders.Server)
    assert(route == null)
    assert(scheme == "http")
    assert(scriptName == "")
    assert(server && server.documentRoot == "." && server.serverRoot == ".")
    assert(sessionID == null)
    assert(status == 200)
    assert(uri == "http://localhost:6700/index.html")
    finalize()
})

let http = new Http
http.get(HTTP + "/index.html")
do { App.eventLoop(10, true) } while(!http.wait())
server.close()
