/*
    Http.fetch tests
 */

let HTTP = App.config.uris.http
let http = new Http
let data = http.fetch(HTTP + "/index.html")
assert(data.length > 0)
