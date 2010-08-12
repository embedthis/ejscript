/*
    Test Controller.read
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    action function sync() {
        let data = new ByteArray
        read(data)
        assert(data == "color=red")
        write("Done")
    }

    action function stream() {
        dontAutoFinalize()
        observe("readable", function (event) {
            let data = new ByteArray
            if (read(data)) {
                assert(data == "Some data")
            } else {
                write("Done")
                finalize()
            }
        })
    }
} 

load("../utils.es")
server = controllerServer(HTTP)


//  Sync read
let http = new Http
http.form(HTTP + "/test/sync", "color=red")
http.wait()
assert(http.response == "Done")
http.close()

//  Stream async I/O
let http = new Http
http.post(HTTP + "/test/stream", "Some data")
http.wait()
assert(http.response == "Done")
http.close()

server.close()
