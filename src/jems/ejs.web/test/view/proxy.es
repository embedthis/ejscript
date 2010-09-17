/*
    Proxy label function
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")

load("../utils.es")

public function proxy(tag, text, options, expected) {
    labelData = { text: text, options: options }
    let http = fetch(HTTP + "/" + tag)
    let response = http.response.trim()
    try {
        //  Validate generated control response
        if (expected is String) {
            assert(response == expected)
        } else if (expected is Array) {
            for each (part in expected) {
                assert(response.contains(part))
            }
        } else {
            for (let [key, value] in expected) {
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
}
