/*
    Form tests
 */

const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)

var http: Http = new Http
http.form(HTTP + "/form.ejs", {name: "Julie", address: "700 Park Ave"})
assert(http.status == 200)

let response = deserialize(http.response)
assert(response.params.name == "Julie")

http.close()
