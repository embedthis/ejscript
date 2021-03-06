/*
    Test request properties inside a web server
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    let j = Request.jsgi
    assert(j)
    assert(j.errors)
    assert(j.version.toString() == "0,3")
    assert(j.multithread)
    assert(!j.multiprocess)
    assert(!j.runonce)
    assert(env)
    assert(env.toJSON() == "{}")

    assert(serverPort == Uri(HTTP).port)
    assert(queryString == "a=b")
    finalize()
})

let http = new Http
http.get(HTTP + "/index.html?a=b")
http.wait()
http.close()
server.close()
