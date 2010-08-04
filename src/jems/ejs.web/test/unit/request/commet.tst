/*
    Test read() - commet style
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)

load("utils.es")

var events

server.observe("readable", function (event, request: Request) {
print("SERVER READ EVENT: " + pathInfo)
    observe(["close", "error", "readable", "writable"], function(event) {
        print("@@@@@@@@@@@@@@@@@@@@@ GOT " + event)
    })
    switch (pathInfo) {
    case "/commet":
print("COMMET")
        observe("readable", function (event) {
print("READABLE")
            let count
            count = read(commetData, -1)
print("COUNT " + count)
            if (!count) {
print("RR FINALIZE")
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

//  MOB - established too late. Should still work
let count = 0
let done = false


http.observe("writable", function (event, h) {
    if (count < 3) {
        http.write("%05d abcaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaxyz\r\n".format(count++))
        // http.flush()
    } else {
        done = true
        http.finalize()
    }
})

http.post(HTTP + "/commet")

while (!done) {
    App.eventLoop(10, true)
}
do { App.eventLoop(10, true) } while(!http.wait())
assert(http.status == 200)

print(http.response)
print(commetData)

server.close()
