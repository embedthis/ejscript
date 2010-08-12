/*
    Test Controller.afterChecker, beforeChecker, removeCheckers
 */
require ejs.web

const PORT = (App.config.test.http_port || 6700)
const HTTP = ":" + PORT

public class TestController extends Controller {
    use namespace action

    function TestController() {
        removeCheckers()
        beforeChecker(authorize, { except: "anyone"} )
        afterChecker(monitor, { only: "secret"} )
    }

    function authorize() {
        events["authorize"] = true
        if (params["key"] != "abracadabra") {
            setStatus(Http.Forbidden)
            events["denied"] = true
            write("Access Denied")
        }
    }
    function monitor() {
        events["monitor"] = true
    }

    action function anyone() {
        write("Welcome Everybody")
    }
    action function secret() {
        write("Top Secret")
    }
} 

load("../utils.es")
server = controllerServer(HTTP)

let events = {}

//  beforeChecker - With key, should be allowed
let http = fetch(HTTP + "/test/secret?key=abracadabra")
assert(http.response == "Top Secret")
assert(events.authorize && events.monitor && !events.denied)
http.close()


//  Without key, should be denied
let http = fetch(HTTP + "/test/secret", Http.Forbidden)
assert(http.response == "Access Denied")
assert(events.authorize && events.monitor && events.denied)
http.close()


events = {}
let http = fetch(HTTP + "/test/anyone")
assert(http.response == "Welcome Everybody")
assert(!events.authorize && !events.monitor && !events.denied)
http.close()

server.close()
