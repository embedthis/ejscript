#!/usr/bin/env ejs

require ejs.web
var address = ":" + (App.config.test.http_port || "6700")
HttpServer.create(address, {documents: "web"})
