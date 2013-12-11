/*
    Test Cascade wrapper
 */

const HTTP = App.config.uris.http

var http: Http = new Http

//  Select app "one"
http.get(HTTP + "/dispach.es?route=cascade&select=one")
assert(http.response == "Hello One")

//  Select app "two"
http.get(HTTP + "/dispach.es?route=cascade&select=two")
assert(http.response == "Hello Two")
http.close()

//  Select catch all app
http.get(HTTP + "/dispach.es?route=cascade")
assert(http.response == "Hello World")
http.close()
