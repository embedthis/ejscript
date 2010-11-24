#!/usr/bin/env ejs

require ejs.web
var address = ":" + (App.config.test.http_port || "6700")
Web.start(address, "web", ".")
