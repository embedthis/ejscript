/*
    Wait tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http

//  Wait on a non-waiting get request
http = new Http
http.get(HTTP + "/index.html")
http.wait()
assert(http.status == 200)
http.close()

//  Wait on a sleeping get request. Sleep blocks for 5 seconds. Wait for 1/2 second via a timeout
http = new Http
http.get(HTTP + "/sleep.ejs")
mark = new Date
assert(http.wait(500) == false)
assert(mark.elapsed > 400)
