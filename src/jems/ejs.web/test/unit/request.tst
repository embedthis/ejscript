/*
    Request unit test
 */

require ejs.web

/*
const HTTP = "http://127.0.0.1:" + ((global.test && test.config.http_port) || 6700)
const HTTPS = "https://127.0.0.1:" + ((global.test && test.config.http_port) || 6743)
const TIMEOUT = 10000

//  Simple creation outside a web server
r = new Request
assert(r)


//  Creation inside a web server
function asyncClient(url) {
    let http: Http = new Http
    http.async = true
    let state
    http.observe(["complete", "readable"], function (event, http) {
        state = event
        if (event == "readable")
            http.read(buf)
    })
    http.get(HTTP + url)
    http.finalize()
    assert(http.status == 200)

    for (let mark = Date();  (!state != "complete" && mark.elapsed < TIMEOUT) {
        App.eventLoop(TIMEOUT - mark.elapsed, 1)
    }
    http.close()
}


function asyncClient(url) {

    let http = Http
    http.open("GET", ENDPOINT + url, function() {
        //  MOB -- problem -- does not progressively read
        http.response()
    }) 


- Accumulates body in response 
- 


var req = new XMLHttpRequest();  
req.open('GET', 'http://www.mozilla.org/', true);  
req.onreadystatechange = function (event) {  
  if (req.readyState == 4) {  
     if(req.status == 200)  
      dump(req.responseText);  
     else  
      dump("Error loading page\n");  
  }  
};  
req.send(null);



    let http: Http = new Http
    http.async = true
    let complete
    http.observe("readable", function (event, http) {
        assert(http.status == 200)
        http.read(buf)
    })
    http.observe("complete", function (event, http) {
        complete = true
    })
    http.get(HTTP + url)
    http.finalize()

    let mark = new Date
    while (!complete && mark.elapsed < TIMEOUT) {
        App.eventLoop(TIMEOUT - mark.elapsed, 1)
    }
    http.close()
}


function wait() {
}

server = new HttpServer(".", "web")
server.observe("readable", function (event: String, request: Request) {
    dump(requeset)
    // request.write("Hello World")
    request.finalize()
})
server.listen(ENDPOINT)
*/

/*
        native var absHome: Uri
        native var authGroup: String
        native var authType: String
        native var authUser: String
        native var chunkSize: Number
        var config: Object
        native var contentLength: Number
        native var contentType: String
        native var cookies: Object
        native var dir: Path
        native var encoding: String
        native var files: Object
        native var headers: Object
        native var home: Uri
        native var host: String

        function get log(): Logger 
        native var method: String
        native var params: Object
        native var pathInfo: String
        native var query: String
        native var referrer: String
        native var remoteAddress: String
        var route: Route
        native var scheme: String
        native var scriptName: String
        native var secure: Boolean
        native var server: HttpServer
        native var session: Session 
        native var sessionID: String
        native var status: Number
        native var timeout: Number
        native var uri: Uri
        native var userAgent: String

        native function observe(name, observer: Function): Void
        native function get async(): Boolean
        native function set async(enable: Boolean): Void
        function cachable(enable: Boolean = false): Void {
        native function close(): Void
        function show(...args): Void {
        native function destroySession(): Void
        function get extension() 
        native function finalize(): Void 
        function flush(dir: Number = Stream.BOTH): Void {}
        native function getResponseHeaders(): Object
        native function header(key: String): String
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void {
        native function removeObserver(name, observer: Function): Void
        function makeUri(where: Object): Uri {
        function setLocation(prefix: String, location: Path): Void {
        function sendResponse(response: Object): Void {
        function setCookie(name: String, options: Object) {
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void
        function setHeaders(headers: Object, overwrite: Boolean = true): Void {
        function setStatus(status: Number): Void {
        native function write(...data): Number
        function writeError(msg: String, code: Number = Http.ServerError): Void {
        function writeHtml(...args): Void

        // JSGI 
        static var jsgi: Object = {
        native var env: Object

        function get input(): Stream {
        function get queryString(): String
        function get serverName(): String {
        function get serverPort(): Number
*/
