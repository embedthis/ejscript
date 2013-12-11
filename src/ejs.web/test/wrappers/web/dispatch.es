/*
    dispatch.es -- Dispatch requests based on query params
 */

require ejs.web

const LOG = Path("access.log")

/*
    Cascade app 1
 */
function app1(request) {
    return {
        status: (request.params.select == "one") ? Http.Ok : Http.NotFound, 
        body: "Hello One",
    } 
}


/*
    Cascade app 2
 */
function app2(request) {
    return {
        status: (request.params.select == "two") ? Http.Ok : Http.NotFound, 
        body: "Hello Two",
    } 
}


/*
    Cascade catch all app and general hello world app
 */
function app(request) {
    return {
        status: Http.Ok,
        body: "Hello World",
    } 
}


/*
    Echo the request properties
 */
function echo(request) {
    return {
        status: Http.Ok,
        headers: { "Content-Type": "application/json" },
        body: serialize(request, {pretty: true}) + "\n",
    } 
}


/* 
   Serve data.md, the ContentType wrapper will set the content type 
 */
function anon(request) {
    request.pathInfo = "data.md"
    return {
        status: Http.Ok,
        body: "Hello Content",
    } 
}


/*
    App that throws an exception
 */
function boom(request) {
    throw new Error("Expected Big Bang")
}


/*
    Mime types for ContentType wrapper
 */
var mimeTypes = {
    "md":   "text/x-markdown",
}

var logger = new Logger("access", LOG)

/*
    Routing map for QueryRoute
 */
var map = {
    "cascade":    Cascade(app1, app2, app),
    "override":   MethodOverride(echo),
    "head":       Head(app),
    "content":    ContentType(anon, {mimeTypes: mimeTypes}),
    "show":       ShowExceptions(boom),
    "log":        CommonLog(app, logger),
    "all":        ShowExceptions(Head(CommonLog(ContentType(MethodOverride(Cascade(app1, app2, app))), logger))),
    "echo":       echo,
    "":           app,
}

function QueryRoute(map) {
    return function (request) {
        let route = request.params.route || ""
        if (!(map[route] is Function)) {
            return errorBody(Http.NotFound, "Cannot find " + route)
        }
        let fun = map[route]
        return fun(request)
    }
}


/*
    Normal style declaration of the form below. QueryRoute dynamically selects a route based on the params.route field..

    exports.app = ShowExceptions(Head(CommonLog(ContentType(MethodOverride(Cascade(app1, app2, app))))))
*/

exports.app = QueryRoute(map)
