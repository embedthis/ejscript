/*
    Wait for a request to complete
 */

const WAIT_TIMEOUT = 30000

public function fetch(url: String, status: Number = Http.Ok): Http {
    let http = new Http
    http.get(url)
    http.wait(WAIT_TIMEOUT)
    if (http.status != status) {
        App.log.debug(0, "STATUS is " + http.status + " expected " + status)
        App.log.debug(0, "RESPONSE is " + http.response)
        assert(http.status == status)
    }
    return http
}
