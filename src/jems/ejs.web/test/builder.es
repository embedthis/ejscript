/*
    Start http server
*/
require ejs.web

let address = App.args[1] || ":6700"
Web.run(address, "web")


//  Convenience routine to start a routing server that will serve a variety of content

function Web.start(address: String, documentRoot: Path = ".", serverRoot: Path = ".", routes = Router.TopRoutes): Void {
    let server: HttpServer = new HttpServer(documentRoot, serverRoot)
    var router = Router(routes)
    server.observe("readable", function (event, request) {

        serve(request, router)

        // or
        router.route(request)
        let app = load(app, request)
        if (app) {
            //  MOB -- process JSGI apps and results
            process(request, app)
        }
    })
    server.listen(address)
    App.eventLoop()
}


//  Run a single JSGI app

function Web.run(address: String, documentRoot: Path = ".", serverRoot: Path = "."): Void {
    require ejs.cjs
    let server: HttpServer = new HttpServer(documentRoot, serverRoot)
    server.observe("readable", function (event, request) {
        let path = request.dir.join(request.pathInfo.slice(1))
        process(Loader.require(path).app)
    })
    server.listen(address)
    App.eventLoop()
}


/*

   Mutators
        ** don't set "app", set name in exports
        Template
        Gzip
        Log
        Trace
        Router

   Generators
        Static
        Script
        Mvc
        Dir

    exports.app = function (request) {
        return {
            status: 200,
            headers: {"Content-Type": "text/plain"},
            body: File("data.txt", "r"),

        }
    }
    exports.app = Log(Route(Trace(Head(app)))

        Template, Mvc, Dir, Static, JSGI

    exports.app = Route.dispatch(app)



MOB -- name clashes with type. What to call JSGI thingies
    function Router(app): Function {
        return function(request) {
            router.route(request)
            var response = app(request)
            /* Post processing */
            return response
        }
    }

*/
