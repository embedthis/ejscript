/*
    Test HttpServer.limits
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || 6700)
load("../utils.es")

server = new HttpServer

server.observe("readable", function (event, request) {
    Timer(10000, function() {
        finalize()
    }).start()
})

server.setLimits({ clients: 10, requests: 1 })

server.listen(HTTP)
var http = new Http
http.get(HTTP)
http.finalize()

//  This request should be rejected
var http2 = new Http
http2.get(HTTP)
http2.finalize()

//  Pump events to allow rhe requests to be serviced
App.eventLoop(250)
try {
    http2.status
    assert(0)
} catch { }

server.close()

