#!/usr/bin/env ejs
/*
    Just for quick testing
 */
require ejs.web

let address = App.args[1] || App.config.test.http_port || ":6700"
let server: HttpServer = new HttpServer(".")

//  MOB -- these routes dont handle /index.html
MyRoutes = [
  { name: "app",     builder: MvcBuilder,                   match: "/blog/", 
                                                            location: { prefix: "/blog/", dir: "mvc-blog" }, },

  { name: "es",      builder: ScriptBuilder,                match: /\.es$/,
        xtrace: { level: 1, options: ["conn", "first", "request"], size: 4096 },
        xlimits: { inactivityTimeout: 30 },
  },
  { name: "ejs",     builder: TemplateBuilder,              match: /\.ejs$/,      module: "ejs.template",  },
  { name: "dir",     builder: DirBuilder,                   match: Router.isDir, },
  { name: "web",     builder: StaticBuilder,                match: /^\/web\//,  },
  { name: "home",    builder: StaticBuilder,                match: /^\/$/,                  redirect: "/web/index.ejs", },
  { name: "ico",     builder: StaticBuilder,                match: /^\/favicon.ico$/,       redirect: "/web/favicon.ico", },
  { name: "list",    builder: MvcBuilder, method: "GET",    match: "/:controller/list",     params: { action: "list", } },
  { name: "create",  builder: MvcBuilder, method: "POST",   match: "/:controller/create",   params: { action: "create", } },
  { name: "edit",    builder: MvcBuilder, method: "GET",    match: "/:controller/edit",     params: { action: "edit", } },
  { name: "update",  builder: MvcBuilder, method: "POST",   match: "/:controller/update",   params: { action: "update", } },
  { name: "destroy", builder: MvcBuilder, method: "POST",   match: "/:controller/destroy",  params: { action: "destroy", } },
  { name: "default", builder: MvcBuilder,                   match: "/:controller/:action",  params: {}, },
  { name: "index",   builder: MvcBuilder, method: "GET",    match: "/:controller",          params: { action: "index", } },
  { name: "static",  builder: StaticBuilder, 
  },
]

// var router = Router(Router.TopRoutes)
var router = Router(MyRoutes)
server.observe("readable", function (event, request) {
    // request.trace(1, ["headers", "request", "response", "body"])
    // request.trace(1, ["headers", "request", "response"])
    // request.trace(1, ["first", "request", "response"])

    // dump(request.limits)
    // request.setLimits({timeout: 120, inactivityTimeout: 60})
    request.setLimits({timeout: 0, inactivityTimeout: 60})

    // App.log.info(request.method, request.uri)
    Web.serve(request, router)
})

App.log.info("Listen on " + address)
server.listen(address)

server.setLimits({clients: 2, requests: 1})
// server.setLimits({transmission: 1024, upload: 10 * 1024})
server.setLimits({timeout: 0, inactivityTimeout: 0})

server.trace(2, ["request", "response", "first", "headers"])
// dump(server.limits)
App.eventLoop()
