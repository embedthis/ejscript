/*
    Test sendResponse, setStatus, setCookie
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {
    switch (pathInfo) {
    case "/sendfile":
        sendFile("../utils.es")
        finalize()
        break

    case "/sendResponse":
        sendResponse({ status: 201, headers: {"Custom": 42}, body: "hello moon"})
        break

    case "/setStatus":
        setStatus(202)
        finalize()
        break

    case "/setCookie":
        setCookie("color", { 
            value: "red", 
            path: "/colortest", 
            domain: ".example.com", 
            expires: Date().future(2 * 3600 * 1000), 
            secure: true 
        })
        finalize()
        break

    case "/hang":
        break

    default:
        writeError(Http.ServerError, "Bad test URI")
    }
})

//  sendResponse
let http = fetch(HTTP + "/sendResponse", 201)
assert(http.status == 201)
assert(http.headers["custom"] == 42)
assert(http.response == "hello moon")
http.close()


//  setStatus
let http = fetch(HTTP + "/setStatus", 202)
assert(http.status == 202)
assert(http.response == "")
http.close()


//  setCookie
let http = fetch(HTTP + "/setCookie")
assert(http.status == 200)
assert(http.response == "")
assert(http.headers["set-cookie"])
let cookie = http.headers["set-cookie"]
assert(cookie.contains("color=red"))
assert(cookie.contains("; path=/colortest"))
assert(cookie.contains("; domain=.example.com"))
assert(cookie.contains("; expires="))
assert(cookie.contains("secure"))
http.close()


//  sendFile
let http = fetch(HTTP + "/sendfile")
assert(http.status == 200)
assert(http.response == Path("../utils.es").readString())
http.close()


// no response
server.setLimits({inactivityTimeout: 1})
let http = fetch(HTTP + "/hang", Http.RequestTimeout)
http.close()

server.close()
