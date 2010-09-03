require ejs.web

let address = ":" + (App.config.test.http_port || "6700")
Web.start(address, "web", ".", Router.Top)
