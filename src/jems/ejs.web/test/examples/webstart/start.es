require ejs.web

let address = ":" + (App.config.test.http_port || "6700")
HttpServer.create(address, {documents: "web"})
