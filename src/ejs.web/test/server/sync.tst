/*
    Test sync HttpServer 
 */

/* FAILING ON FARM
require ejs.web

const HTTP = App.config.uris.http
load("../utils.es")

var success
var w = new Worker
w.onmessage = function(e) {
    let msg = e.data
    let http = fetch(HTTP + "/")
    assert(http.response == "Hello World")
    assert(http.status == Http.Ok)
    success = true
}

w.eval('
    //  Run web serer inside a worker so we can do a sync server
    require ejs.web
    const HTTP = App.config.uris.http
    let server = new HttpServer
    server.async = false
    server.listen(HTTP)
    postMessage("listening")

    //  Sync server. Loop waiting for requests
    while (request = server.accept()) {
        request.write("Hello World")
        request.finalize()
        //  Exit
        break
    }
    server.close()
')

//  Wait for worker to complete
Worker.join(null, 4000)
assert(success)
*/
