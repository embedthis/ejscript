/*
    Routed Http server

    This demonstrates the various kinds of rouces the Router class can support. Typically the standard routes provided
    by the Router class Router.Restful, Router.Default and Router.Legacy are sufficient. However, users can create and 
    modify their own routes.

    Usage: ejs routed-server.es [PORT]
    Client:
        browse to: http://localhost:PORT/index.html
        browse to: http://localhost:PORT/test.ejs
        browse to: http://localhost:PORT/hello.ejs
*/

require ejs.web

let server: HttpServer = new HttpServer(".", "./web")

/*
    Define the routes. The routes are matched in order. The router table fields are:
        name    - Defines a name for reference and debugging purposes 
        type    - Defines the request type. The Web and other custom routes switch on this field
        match   - Regular expression or tokenized string specifying the URL format
        params  - Request parameters to define in Request.params[]. May utilize :token values from the match field.
*/
public static var RestfulRoutes = [
  /* JSGI pages with a ".es extension */
  { name: "es",      type: "es",                    match: /^\/web\/.*\.es$/   },

  /* Stand-alone templated ".ejs" pages */
  { name: "ejs",     type: "ejs",                   match: /^\/web\/.*\.ejs$/  },

  /* Static web content in the "web" directory */ 
  { name: "web",     type: "static",                match: /^\/web\//  },

  /* Home page - redirect to "web/index.ejs" */
  { name: "home",    type: "static",                match: /^\/$/, redirect: "/web/index.ejs" },

  /* Custom redirection for favicon.ico */
  { name: "ico",     type: "static",                match: /^\/favicon.ico$/, redirect: "/web/favicon.ico" },

  /* Routes for an MVC application in the "admin" directory that uses a "/control" URL prefix */
  { name: "app",     type: "mvc",                   match: "/admin/", location: { prefix: "/control", dir: "admin" },

  /* Rewrite the "old.html" URL to "new.html" */
  { name: "rewrite", type: "",                      match: "/web/old.html", 
                                                    rewrite: function (request) { request.pathInfo = "/web/new.html" }},
//  MOB -- what about the blog app?

  /* MVC restful routes */
  { name: "new",     type: "mvc", method: "GET",    match: "/:controller/new",       params: { action: "new" } },
  { name: "edit",    type: "mvc", method: "GET",    match: "/:controller/:id/edit",  params: { action: "edit" } },
  { name: "show",    type: "mvc", method: "GET",    match: "/:controller/:id",       params: { action: "show" } },
  { name: "update",  type: "mvc", method: "PUT",    match: "/:controller/:id",       params: { action: "update" } },
  { name: "delete",  type: "mvc", method: "DELETE", match: "/:controller/:id",       params: { action: "delete" } },
  { name: "default", type: "mvc",                   match: "/:controller/:action",   params: {} },
  { name: "create",  type: "mvc", method: "POST",   match: "/:controller",           params: { action: "create" } },
  { name: "index",   type: "mvc", method: "GET",    match: "/:controller",           params: { action: "index" } },

  /* Handle files with a ".js" extension as ".es files"
  { name: "js",      type: "es",                     match: /\.es$/ },

  /* Catch all */
  { name: "catch",  type: "es",                                                     params: { action: "index" } },
]

var router = Router(routes)
server.on("readable", function (event, request) {
    Web.serve(request, router)
})

server.listen("127.0.0.1:7000")
print("Listening on port 127.0.0.1:7000")
App.eventLoop()


/*
    Other kinds of routes are shown below:

MOB -- these need to be tested

var MoreRoutes = [
    {   
        //  Match by function 
        name: "secure",    
        method: "GET",    
        match: function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }
        }, 
    },
    {   
        //  Match with regular expression. Options extracts sub-matches via $1
        name: "regexp",
        method: "GET",    match: /^\/Dash-((Mini)|(Full))/, 
        params: { 
            controller: "post", 
            action: "list", 
            kind: "$1",
        },
    },
    {   
        //  Simple string match
        name: "blog",
        method: "",
        match: "/blog", 
        params: { 
            controller: "post",
        }, 
        subroute: { 
            //  Nested match
            name: "posting", 
            method: "",
            match: "/more/:action/:id", 
            params: { 
                custom: "123",
            } ,
        },
    }, 
    {   
        //  Match with format token parsing
        name: "format",  
        method: "GET",
        match: "/:controller.:format",
        params: { 
            action: "index",
        },
    },
    {   
        //  Match with function, rewrite and continue
        name: "continue",  
        method: "",
        match: function (request) {
            if (request.pathInfo.startsWith("/cont/")) {
                request.pathInfo.trimStart("/cont")
            }
            return false
        },
    },
    {   
        //  Match with rewriting
        name: "rewrite",  
        method: "",
        match: "/app/",
        rewrite: function (request) { 
            request.scriptName = "/app"
            request.pathInfo.trimStart("/app")
        },
    },
]

*/
