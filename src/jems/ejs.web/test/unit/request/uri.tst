/*
    Test makeUri
 */
require ejs.web

const PORT = App.config.test.http_port || 6700
const HTTP = ":" + PORT

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.observe("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/uri":
        let u = makeUri("/someUri")
        assert(u == "http://127.0.0.1:" + PORT + "/someUri")
        let u = makeUri({scheme: "https", host: "example.com", port: 8080, path: "/some.html", query: "color=red", 
            reference: "42"})
        assert(u == "https://example.com:8080/some.html#42?color=red")
        finalize()
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})


//  makeUri
let http = fetch(HTTP + "/uri")
assert(http.status == 200)
http.close()

server.close()
