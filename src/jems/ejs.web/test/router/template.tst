/*
    Router Templates
 */
require ejs.web

const HTTP = ":" + (App.config.test.http_port || "6700")
server = new HttpServer
server.listen(HTTP)

let router

server.on("readable", function (event, request: Request) {
    let app = router.route(request)
dump(request)
    write(serialize(request) + "\n")
    finalize()
})

function builder(): Function {
    return function(request): Object {
        return {body: "Unused"}
    }
}

function test(url: String, method: String = "GET"): Object {
    let http = new Http
    http.get(HTTP + url)
    http.wait(30000)
print(http.response)
    assert(http.status == Http.Ok)
    return deserialize(http.response)
}


/****OK
//  /path/next/last

router = new Router
router.add("/path/next/last", {run: builder})
let r = test("/path/next/last")
assert(r.pathInfo == "/path/next/last")
assert(Object.getOwnPropertyCount(r.params) == 0)
*/


//  With tokens

/*
router = new Router
router.add("/{color}/{flavor}/{temp}", {name: "test", run: builder})
let r = test("/red/mild/100")
dump(r)
assert(Object.getOwnPropertyCount(r.params) == 0)
assert(r.params.color == "red")
assert(r.params.flavor == "mild")
assert(r.params.temp == "100")
*/

server.close()
