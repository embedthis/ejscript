/*
    Ejs script to serve web requests on port
*/
require ejs.web

let server: HttpServer = new HttpServer(".", "./web")

var routes = [
    {   //  Route for the "junk" MVC application
        name: "junk", 
        type: "mvc", 
        match: "/junk/", 
        location: {prefix: "/junk", dir: "junk" },
        xrewrite: function (request) {
            request.setApp("/junk", "junk")
            return true
        },
        xurimaker: function (options, request) {
            return "/fixed/path"
        },
    },
    {   name: "es",  type: "es",  match: /\.es$/,  threaded: false   },       // Route for JSGI *.es apps
    {   name: "ejs", type: "ejs", match: /\.ejs$/,                  },       // Route for Template *.ejs web pages

    {   name: "catchall", type: "es", match: "/:controller/:action" },
]

var router = Router(routes)
server.addListener("readable", function (event, request) {
    Web.serve(request, router)
})

server.listen("127.0.0.1:7000")
print("Listening on port 127.0.0.1:7000")
App.serviceEvents()


/*

//  MOB -- merge TestRoutes with routes below and create unit tests

var TestRoutes = [
    {   //  Match by function 
        name: "secure",    
        method: "GET",    
        match: function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }
        }, 
        params: { 
            controller: "post", 
            action: "list"
        }, 
    },
    {   //  Match with regular expression. Options extracts sub-matches via $1
        name: "regexp",
        method: "GET",    match: /^\/Dash-((Mini)|(Full))/, 
        params: { 
            controller: "post", 
            action: "list", 
            kind: "$1",
        },
    },
    {   //  Simple string match
        name: "blog",
        method: "",
        match: "/blog", 
        params: { 
            controller: "post",
        }, 
        subroute: { //  Nested match
            name: "posting", 
            method: "",
            match: "/more/:action/:id", 
            params: { 
                custom: "123",
            } ,
        },
    }, 
    {   //  Match with format token parsing
        name: "format",  
        method: "GET",
        match: "/:controller.:format",
        params: { 
            action: "index",
        },
    },
    {   //  Match with function, rewrite and continue
        name: "continue",  
        method: "",
        match: function (request) {
            if (request.pathInfo.startsWith("/cont/")) {
                request.pathInfo.trimStart("/cont")
            }
            return false
        },
    },
    {   //  Match with rewriting
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



/*OK - low level async server

require ejs.web.template
require ejs.cjs

server.addListener("readable", function (event, request) {
    print("NEW REQUEST scriptName" + request.scriptName)

    //  TODO
    // request.sendResponse({ status: 200, headers: {"Content-Length": 11}, body: "Some Data\r\n"})

    //  TODO - should these be setters?
    request.status = 200
    request.contentLength = 11
    request.setHeaders({"Content-Type": "text/plain"})

    request.addListener("readable", function (event, request) {
        print("READABLE EVENT")
        assert(request.contentLength > 0)
        let data = new ByteArray
        if (request.read(data) == null) {
            print("EOF on read")
        }
        print("READ " + data.available)
    })

    request.addListener("writable", function (event, request) {
        print("WRITABLE EVENT")
        request.write("Some Data\r\n")
        request.finalize()
    })

    //  TODO - should this be close or complete
    request.addListener("complete", function (event, request) {
        print("COMPLETE EVENT")
    })

    //  TODO - should add an error event
    request.addListener("error", function (event, request) {
        print("ERROR EVENT")
    })
    print("DONE - accept")
})


server.listen("127.0.0.1:7777")
App.serviceEvents()
*/

