/*
    Test read() - commet style
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
const COUNT = 1000

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.observe("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/commet":
        observe("readable", function (event) {
            if (read(commetData, -1) == null) {
                write("Hello World")
                finalize()
            }
        })
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})


// read - commet-style
var commetData = new ByteArray
let http = new Http
let done = 0
http.async = true
http.observe("writable", function (event, h) {
    if (done < 1000) {
        http.write("%05d abcaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaxyz\r\n".format(done++))
        http.flush()
    } else {
        http.finalize()
    }
})
http.post(HTTP + "/commet")

do { App.eventLoop(10, true) } while(!http.wait())

assert(http.status == 200)
assert(http.response == "Hello World")
assert(commetData.toString().contains("0001 abc"))
assert(commetData.toString().contains("0099 abc"))
assert(commetData.available == 70000)

server.close()
