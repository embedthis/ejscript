/*
    Test write-error() 
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/show-client":
        config.log.showErrors = true
        writeError(Http.ServerError, "Test Error Message")
        break

    case "/no-show-client":
        config.log.showErrors = false
        writeError(Http.ServerError, "Test Error Message")
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

//  Without client message
let http = fetch(HTTP + "/no-show-client", Http.ServerError)
assert(http.status == Http.ServerError)
assert(http.response == "")
http.close()

//  With full client message
let http = fetch(HTTP + "/show-client", Http.ServerError)
assert(http.status == Http.ServerError)
assert(http.response.contains("DOCTYPE"))
assert(http.response.contains("<head>"))
assert(http.response.contains("<body>"))
assert(http.response.contains("Test Error Message"))
assert(http.response.contains("showErrors"))
http.close()
server.close()
