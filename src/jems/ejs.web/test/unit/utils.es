/*
    Fetch with event servicing
 */
public function fetch(url: String, status: Number = Http.Ok): Http {
    let http = new Http
    http.get(url)
    do { App.eventLoop(10, true) } while(!http.wait())
    if (http.status != status) {
        App.log.debug(0, "STATUS is " + http.status + " expected " + status)
        App.log.debug(0, "RESPONSE is " + http.response)
        assert(http.status == status)
    }
    return http
}
