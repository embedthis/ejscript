/*
    Test read() - commet style
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
const COUNT = 1000

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
print("READABLE" + pathInfo)
    switch (pathInfo) {
    case "/commet":
print("DONT")
        dontAutoFinalize()
print("SETUP ON")
        on("readable", function (event) {
            // read(commetData, -1) == null)
            let len = read(commetData, -1)
print("READ " + len + " total available " + commetData.available) 
            if (read(commetData, -1) == null) {
print("EOF - write HELLO WORLD")
                write("Hello World")
                finalize()
            }
        })
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})
print("STARTED")


// read - commet-style
var commetData = new ByteArray
let http = new Http
let done = 0
let total = 0
http.async = true
http.on("writable", function (event, h) {
    //  MOB - was 1000
    if (done < 1000) {
        let s = "%05d abcaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaxyz\r\n".format(done++)
        http.write(s)
        http.flush()
        total += s.length
print("WRITE " + s.length + " total " + total)
    } else {
print("FINALIZE " + done)
        http.finalize()
    }
})
print("POSTING")
http.post(HTTP + "/commet")
print("WAITING")
http.wait()
print("WAITED")

assert(http.status == 200)
assert(http.response == "Hello World")
/*
assert(commetData.toString().contains("0001 abc"))
assert(commetData.toString().contains("0099 abc"))
assert(commetData.available == 70000)
*/

server.close()
