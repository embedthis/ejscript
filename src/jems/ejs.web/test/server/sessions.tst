/*
    Test HttpServer session limits and timeouts
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || 6700)
load("../utils.es")

server = new HttpServer

server.on("readable", function (event, request) {
    session["anything"] = 42
    request.finalize()
})


//  Set max sessions to 2 and a timeout of 1 second
assert(server.sessions == null)
server.setLimits({ sessions: 2, sessionTimeout: 1 })

server.listen(HTTP)

//  Fetch and create a new session object per request
assert(Object.getOwnPropertyCount(server.sessions) == 0)
http = fetch(HTTP + "/")
http.reset()
http = fetch(HTTP + "/")
assert(Object.getOwnPropertyCount(server.sessions) == 2)

//  Wait for old sessions to be pruned 
App.sleep(1100)
assert(Object.getOwnPropertyCount(server.sessions) == 0)

server.close()

