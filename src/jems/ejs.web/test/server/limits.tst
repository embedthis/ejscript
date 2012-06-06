/*
    Test HttpServer.limits
 */
require ejs.web

const HTTP = App.config.uris.http
load("../utils.es")

server = new HttpServer

server.on("readable", function (event, request) {
    Timer(10000, function() {
        finalize()
    }).start()
})

server.setLimits({ requests: 1 })

server.listen(HTTP)
let http = new Http
http.get(HTTP)
http.close()

//  This request should be rejected
http = new Http
http.get(HTTP)

//  Pump events to allow the requests to be serviced
App.run(250)
try {
    //  This should throw because the request limit will prevent the connection from succeeding.
    http.status
    assert(0)
} catch { }

http.close()
server.close()
