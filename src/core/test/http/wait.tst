/*
    Wait tests
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

var http: Http

//  Wait on a non-waiting get request
http = new Http
http.get(HTTP + "/index.html")
http.wait()
assert(http.status == 200)
http.close()

//  Wait on a sleeping get request. The sleep page takes 5 seconds. Wait for 1/3 second via a timeout
http = new Http
http.get(HTTP + "/sleep.ejs")
mark = new Date
assert(http.wait(333) == false)
assert(mark.elapsed > 300)
