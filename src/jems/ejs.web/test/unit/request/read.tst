/*
    Test read()
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
// server.trace(0, ["headers", "first", "body", "request", "response"])
load("../utils.es")

server.observe("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/form":
        assert(params.color == "red")
        assert(params.shape == "square")
        let ba = new ByteArray
        assert(read(ba) == 22)
        assert(ba == "color=red&shape=square")
        finalize()
        break

    case "/post-sync":
        observe("readable", function (event) {
            let ba = new ByteArray
            if (read(ba)) {
                assert(ba == "Some data")
            } else {
                finalize()
            }
        })
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})


//  form 
let http = new Http
http.form(HTTP + "/form", { color: "red", shape: "square" })
do { App.eventLoop(10, true) } while(!http.wait())
assert(http.status == 200)
assert(http.response == "")
http.close()


//  post 
let http = new Http
http.post(HTTP + "/post-sync", "Some data")
do { App.eventLoop(10, true) } while(!http.wait())
assert(http.status == 200)
assert(http.response == "")
http.close()

server.close()
