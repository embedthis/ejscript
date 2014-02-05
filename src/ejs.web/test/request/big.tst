/*
    One big Request.write()
 */
require ejs.web

const HTTP = App.config.uris.http

//  This unit test does not work due to using a sync client. The server-side write blocks waiting for the client
const COUNT = 4

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

//  About 256K of data
let data = new ByteArray
let written = 0
for (i in COUNT) {
    written += data.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: " + i + "\n")
}

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/single":
        write(data)
        finalize()
        break

/* FUTURE
    case "/multiple":
        dontAutoFinalize()
        on("writable", function (event, request) {
            //  To write in smaller chunks from a single byte array
            writeBlock(data, -1, System.Bufsize)
        })
        close()
        break
 */

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})


//  Single write by the server

let http = new Http
http.afetch("GET", HTTP + "/single", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
assert(http.response.length == written)
http.close()


/* FUTURE
//  Multiple writes by the server
let http = new Http
http.afetch("GET", HTTP + "/multiple", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
// print(http.response.length)
assert(http.response.length == written)
// print(http.response)
http.close()
*/

server.close()
