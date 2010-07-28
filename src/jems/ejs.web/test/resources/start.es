#!/usr/bin/env ejs
/*
    General http server to test individual resource pages
 */
require ejs.web

let address = ":" + (App.config.test.http_port || "6700")
let server: HttpServer = new HttpServer("web")
let routeTable = [
  { name: "app",     builder: MvcBuilder,                   match: "/blog/", 
                                                            location: { prefix: "/blog/", dir: "mvc-blog" }, },
  { name: "es",      builder: ScriptBuilder,                match: /\.es$/   },
  { name: "ejs",     builder: TemplateBuilder,              match: /\.ejs$/,      module: "ejs.web.template"  },
  { name: "dir",     builder: DirBuilder,                   match: Router.isDir },
  { name: "web",     builder: StaticBuilder,                match: /^\/web\//  },
  { name: "home",    builder: StaticBuilder,                match: /^\/$/,                  redirect: "/web/index.ejs" },
  { name: "ico",     builder: StaticBuilder,                match: /^\/favicon.ico$/,       redirect: "/web/favicon.ico" },
  { name: "list",    builder: MvcBuilder, method: "GET",    match: "/:controller/list",     params: { action: "list" } },
  { name: "create",  builder: MvcBuilder, method: "POST",   match: "/:controller/create",   params: { action: "create" } },
  { name: "edit",    builder: MvcBuilder, method: "GET",    match: "/:controller/edit",     params: { action: "edit" } },
  { name: "update",  builder: MvcBuilder, method: "POST",   match: "/:controller/update",   params: { action: "update" } },
  { name: "destroy", builder: MvcBuilder, method: "POST",   match: "/:controller/destroy",  params: { action: "destroy" } },
  { name: "default", builder: MvcBuilder,                   match: "/:controller/:action",  params: {} },
  { name: "index",   builder: MvcBuilder, method: "GET",    match: "/:controller",          params: { action: "index" } },
  { name: "static",  builder: StaticBuilder, },
]

var router = Router(routeTable)
server.observe("readable", function (event, request) {
    App.log.info(request.method, request.uri)
    Web.serve(request, router)
})

App.log.info("Listen on " + address)
server.listen(address)
App.eventLoop()
