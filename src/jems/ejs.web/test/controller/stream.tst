/*
    Stream I/O using Controller.write
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function big() {
        dontAutoFinalize()
        let count = 0
        observe("writable", function (event) {
            if (count++ < 1000) {
                write("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa: " + count + "\n")
            } else {
                finalize()
            }
        })
    }
} 

load("../utils.es")
server = controllerServer(HTTP)


//  Sync read

let http = new Http
http.fetch("GET", HTTP + "/test/big", null)
App.waitForEvent(http, "close", 30000)
assert(http.status == 200)
assert(http.response.length > 0)
assert(http.response.length > 0)
assert(http.response.contains("aa: 1000"))
http.close()

server.close()
