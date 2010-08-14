/*
    One big Request.write()
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
const COUNT = 4096

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

//  About 256K of data
let data = new ByteArray
for (i in COUNT) {
    data.write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: " + i + "\n")
}

server.observe("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/single":
        write(data)
        finalize()
        break

/* FUTURE
    case "/multiple":
        dontAutoFinalize()
        observe("writable", function (event, request) {
            //  To write in smaller chunks from a single byte array
            writeBlock(data, -1, System.Bufsize)
        })
        finalize()
        break
 */

    default:
        writeError(Http.ServerError, "Bad test URI")
    }

})


//  Single write by the server

let http = new Http
http.fetch("GET", HTTP + "/single", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
// print(http.response.length)
assert(http.response.length == 273322)
// print(http.response)
assert(http.response.contains("aa: 4095"))
http.close()


/* FUTURE
//  Multiple writes by the server

let http = new Http
http.fetch("GET", HTTP + "/multiple", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
print(http.response.length)
assert(http.response.length == 273322)
print(http.response)
assert(http.response.contains("aa: 4095"))
http.close()
*/

server.close()
