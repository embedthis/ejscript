require ejs.web
require ejs.web.template
require ejs.cjs

let address = App.args[1] || ":6700"
let server: HttpServer = new HttpServer(".", "web")

server.observe("readable", function (event, request) {
    print("NEW REQUEST scriptName" + request.scriptName)

    //  TODO
    // request.sendResponse({ status: 200, headers: {"Content-Length": 11}, body: "Some Data\r\n"})

    //  TODO - should these be setters?
    //  MOB - try without a content length. Chunking should still work
    request.status = 200
    request.setHeaders({
        // "Content-Length", 11,
        "Content-Type": "text/plain",
    })
    request.observe("readable", function (event, request) {
        let data = new ByteArray
        if (request.read(data)) {
            request.write(data)
        } else {
            request.finalize()
        }
    })
    request.observe("writable", function (event, request) {
    /*
        request.write("Some Data\r\n")
        request.finalize()
    */
    })
    request.observe("close", function (event, request) {
        print("CLOSE EVENT")
    })
    request.observe("error", function (event, request) {
        print("ERROR EVENT")
    })
})


server.listen(address)
App.log.info("Listen on " + address)
App.eventLoop()


