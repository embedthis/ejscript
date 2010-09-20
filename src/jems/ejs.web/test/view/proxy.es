/*
    Proxy label function
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

public var proxyData

const WAIT_TIMEOUT = 30000
var cookie

public function fetch(url: String, status: Number = Http.Ok): Http {
    let http = new Http
    http.setCookie(cookie)
    http.get(url)
    http.wait(WAIT_TIMEOUT)
    if (http.status != status) {
        App.log.debug(0, "STATUS is " + http.status + " expected " + status)
        App.log.debug(0, "RESPONSE is " + http.response)
        assert(http.status == status)
    }
    if (http.sessionCookie) {
        cookie = http.sessionCookie
    }
    return http
}


public function proxy(tag, ...args): Http {
    expected = args.pop()
    proxyData = args
    let http = fetch(HTTP + "/" + tag)
    let response = http.response.trim()
    try {
        //  Validate generated control response
        if (expected is String) {
            assert(response == expected)
        } else if (expected is Array) {
            for each (part in expected) {
                if (!(response.contains(part))) {
                    print("\nResponse does not contain: " + part + "\n")
                }
                assert(response.contains(part))
            }
        } else if (expected != null) {
            for (let [key, value] in expected) {
                if (!(response.contains(value))) {
                    print("\nResponse does not contain: " + value + "\n")
                }
                assert(response.contains(value))
            }
        }
    } catch (e) {
        print("Assertion failure:")
        print("Expected: \"" + expected + "\"")
        print("Actual:   \"" + response + "\"")
        print(response)
        print(response.length, expected.length)
        throw e
    }
    http.close()
    return http
}
