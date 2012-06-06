/*
    Form tests
 */

const HTTP = App.config.uris.http
var http: Http = new Http

http.form(HTTP + "/form.ejs", {name: "Julie", address: "700 Park Ave"})
assert(http.status == 200)
let response
try {
    response = deserialize(http.response)
} catch (e) {
    print(e)
    print("CATCH RESPONSE " + http.response)
    print("STATUS " + http.status)
}
assert(response.params.name == "Julie")
http.close()
