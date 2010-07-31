#!/usr/bin/env ejs

require ejs.web
var address = ":" + (App.config.test.http_port || "6700")

/*
function MyBuilder(request) {
    dump(params)
    request.filename = "web/content-type.es"
    return Loader.require(request.filename, request.config).app
}
*/

Web.start(address, "web", ".")

/* , [
  { name: "script", builder: ScriptBuilder, match: /\.es$/, },
  { name: "xx", builder: MyBuilder, match: /\.xx$/, },
])
*/
