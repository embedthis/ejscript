/*
    Router unit tests helper
 */

const HTTP = ":" + (App.config.test.http_port || "6700")

public function builder(): Function {
    return function(request): Object {
        return {body: "Unused"}
    }
}

/*
      Client HTTP request helper. Return the response as a serialized object
 */
public function test(url: String, method: String = "GET"): Object {
    let http = new Http
    http.get(HTTP + url)
    http.wait(30000)
    assert(http.status == Http.Ok)
    return deserialize(http.response)
}

