/*
    Test request properties inside a web server
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

server = new HttpServer
server.listen(HTTP)

server.observe("readable", function (event, request: Request) {
    let j = Request.jsgi
    assert(j)
    assert(j.errors)
    assert(j.version.toString() == "0,3")
    assert(j.multithread)
    assert(!j.multiprocess)
    assert(!j.runonce)
    assert(env)
    assert(env.toJSON() == "{}")

    assert(serverPort == PORT)
    assert(queryString == "a=b")
    finalize()
})

let http = new Http
http.get(HTTP + "/index.html?a=b")
do { App.eventLoop(10, true) } while(!http.wait())
server.close()
