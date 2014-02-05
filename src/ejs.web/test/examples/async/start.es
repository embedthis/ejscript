/*
    Sync http server
 */
require ejs.web

const HTTP = App.config.uris.http
let server: HttpServer = new HttpServer({documents: "../../web"})

server.on("readable", function (event, request) {
    server.serve(request)
/*
    switch (request.pathInfo) {
    case "/test1":
        break
    default:
        ;
    }

    print("NEW REQUEST scriptName" + request.scriptName)

    //  TODO
    // request.sendResponse({ status: 200, headers: {"Content-Length": 11}, body: "Some Data\r\n"})

    //  TODO - should these be setters?
    //  TODO - try without a content length. Chunking should still work
    request.status = 200
    request.setHeaders({
        // "Content-Length", 11,
        "Content-Type": "text/plain",
    })
    request.on("readable", function (event, request) {
        let data = new ByteArray
        if (request.read(data)) {
            request.write(data)
        } else {
            request.finalize()
        }
    })
    request.on("writable", function (event, request) {
        // request.write("Some Data\r\n")
        //  request.finalize()
    })
    request.on("close", function (event, request) {
        print("CLOSE EVENT")
    })
    request.on("error", function (event, request) {
        print("ERROR EVENT")
    })
*/
})

server.listen(HTTP)
App.log.info("Listen on " + HTTP)
App.run()
