/*
    Test headers
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

server = new HttpServer
server.listen(HTTP)

server.observe("readable", function (event, request: Request) {

    //  Request headers
    count = Object.getOwnPropertyCount(headers)
    assert(count == 4)
    assert(headers && headers.host && headers.date && headers.connection && headers["user-agent"])
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


    //  Response headers
    assert(responseHeaders && responseHeaders.Server)
    assert(Object.getOwnPropertyCount(responseHeaders) == 1)
    responseHeaders = {}
    assert(Object.getOwnPropertyCount(responseHeaders) == 0)

    // setHeader
    setHeader("Content-Type", "text/plain")
    assert(Object.getOwnPropertyCount(responseHeaders) == 1)
    setHeader("Items", "one")
    assert(responseHeaders.Items == "one")
    setHeader("Items", "two", false)
    assert(responseHeaders.Items == "one, two")
    setHeader("Items", "three", true)
    assert(responseHeaders.Items == "three")

    setHeaders({More: "four", Weather: "sunny"})
    assert(Object.getOwnPropertyCount(responseHeaders) == 4)
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
do { App.eventLoop(10, true) } while(!http.wait())

server.close()
