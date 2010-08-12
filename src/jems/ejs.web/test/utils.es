/*
    Fetch a URI with event servicing
 */
public function fetch(url: String, status: Number = Http.Ok): Http {
    let COUNT 1000
    let http = new Http
    http.get(url)
    for (i = 0; i < COUNT; i++) {
        App.eventLoop(10, true)
        if (http.wait()) {
            break
        }
    }
    if (!http.wait()) {
        throw new Error("Timeout waiting for " + url)
    }
    if (http.status != status) {
        App.log.debug(0, "STATUS is " + http.status + " expected " + status)
        App.log.debug(0, "RESPONSE is " + http.response)
        assert(http.status == status)
    }
    return http
}
