/*
    Start http server
*/
require ejs.web

let address = App.args[1] || ":6700"

//  Start single JSGI app
Web.run(address, "web")

//  Start web server
Web.start(address, "web", ".", Router.TopRoutes)

/*
   Applications
       is a JavaScript function. It takes exactly one argument, the request, and returns a JavaScript Object 
       containing three properties: the status, the headers, and the body.
   Middleware
       is a function that takes at least one other web application and returns another function which is also a 
       web application.

   Middleware - Mutators
        exports.Name = function(app) {}
        Template
        Gzip
        Log
        Trace
        Router

   Apps - Generators
        exports.app = function(request) {}
        Static
        Script
        Mvc
        Dir

    Builders
        exports.app = Name(request)
        TemplateBuilder 

    exports.app = function (request) {
        return {
            status: 200,
            headers: {"Content-Type": "text/plain"},
            body: File("data.txt", "r"),

        }
    }
    exports.app = Log(Route(Trace(Monitor(Head(app))))

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
