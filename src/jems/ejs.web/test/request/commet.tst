/*
    Test read() - commet style
 */
require ejs.web

const HTTP = App.config.uris.http
const COUNT = 1000

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/commet":
        dontAutoFinalize()
        on("readable", function (event) {
            // read(commetData, -1) == null)
            let len = read(commetData, -1)
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
let total = 0
http.async = true
http.on("writable", function (event, h) {
    if (done < 1000) {
        let s = "%05d abcaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaxyz\r\n".format(done++)
        http.write(s)
        http.flush()
        total += s.length
    } else {
        http.finalize()
    }
})
http.post(HTTP + "/commet")
http.wait()

assert(http.status == 200)
assert(http.response == "Hello World")
assert(commetData.toString().contains("0001 abc"))
assert(commetData.toString().contains("0099 abc"))
assert(commetData.length == 70000)
http.close()

server.close()
