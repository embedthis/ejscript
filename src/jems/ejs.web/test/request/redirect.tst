/*
    Test redirect
 */
require ejs.web

const PORT = App.config.test.http_port || 6700
const HTTP = ":" + PORT

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/redirect-string":
        redirect("/elsewhere/", Http.MovedPermanently)
        break

    case "/redirect-object":
        redirect({ host: "example.com", port: 8080, path: "/home", query: "color=blue"}, Http.MovedTemporarily)
        break

    case "/redirect/relative":
        redirect("../../absolute/dir.html")
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
    finalize()
})


//  redirect-string
let http = fetch(HTTP + "/redirect-string", Http.MovedPermanently)
assert(http.status == Http.MovedPermanently)
assert(http.header("location") == "http://localhost:6700/elsewhere/")
http.close()

//  redirect-object
let http = fetch(HTTP + "/redirect-object", Http.MovedTemporarily)
assert(http.status == Http.MovedTemporarily)
assert(http.header("Location") == "http://example.com:8080/home?color=blue")
http.close()

//  redirect-relative
let http = fetch(HTTP + "/redirect/relative", Http.MovedTemporarily)
assert(http.status == Http.MovedTemporarily)
assert(http.header("Location") == "http://localhost:6700/absolute/dir.html")

server.close()
