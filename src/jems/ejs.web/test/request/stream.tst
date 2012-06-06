/*
    Streamed Request.write()
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    dontAutoFinalize()
    let count = 0
    on("writable", function (event) {
        if (count++ < 1000) {
            write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: " + count + "\n")
        } else {
            finalize()
        }
    })
})


//  Async fetch with async writing

let http = new Http
http.fetch("GET", HTTP + "/test/big", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
assert(http.response.length > 0)
assert(http.response.contains("aa: 1000"))
http.close()

server.close()
