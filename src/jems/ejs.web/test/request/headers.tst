/*
    Test headers
 */
require ejs.web

const HTTP = App.config.uris.http

server = new HttpServer
server.listen(HTTP)
load("../utils.es")

server.on("readable", function (event, request: Request) {

    //  Request headers
    count = Object.getOwnPropertyCount(headers)
    assert(count == 5)
    assert(headers && 
        header("Host") && header("Date") && header("Connection") && header("User-Agent") && header("Accept-Ranges"))
    headers["Additional-Header"] = 1234
    assert(headers["Additional-Header"] == 1234)
    assert(Object.getOwnPropertyCount(headers) == count + 1)

    //  Test case insensitivity for client supplied headers
    assert(header("date"))
    assert(header("Date"))
    assert(header("Additional-Header") == 1234)

    //  Complete replacement of request headers
    headers = { one: 1, two: 2}
    assert(headers.one && headers.two)
    assert(Object.getOwnPropertyCount(headers) == 2)

    //  Response headers (Accept-Ranges)
    assert(Object.getOwnPropertyCount(responseHeaders) == 1)

    // setHeader
    setHeader("Content-Type", "text/plain")
    assert(Object.getOwnPropertyCount(responseHeaders) == 2)
    setHeader("Items", "one")
    assert(responseHeaders.Items == "one")
    setHeader("Items", "two", false)
    assert(responseHeaders.Items == "one, two")
    setHeader("Items", "three", true)
    assert(responseHeaders.Items == "three")

    setHeaders({More: "four", Weather: "sunny"})
    assert(Object.getOwnPropertyCount(responseHeaders) == 5)
    assert(responseHeaders["Content-Type"] == "text/plain")
    assert(responseHeaders.Items == "three")
    assert(responseHeaders.More == "four")
    assert(responseHeaders.Weather == "sunny")

    //  Case sensitive
    assert(responseHeaders.weather == null)

    finalize()
})

let http = new Http
http.get(HTTP + "/index.html")
http.wait()

server.close()
