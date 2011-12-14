/*
    Demonstrate the Socket API. This is a mini (non-production) HTTP server.
    NOTE: Real web servers support Keep-Alive, Chunking, Ranged requests, PUT, DELETE etc.
    This web server just responds to simple GET requests.
 */

var methods = { "DELETE", "GET", "OPTIONS", "POST", "PUT", "TRACE" }

class Response {
    private var conn: Socket
    private var request: Request
    private var status: Number = 200
    private var headers: Object = {}

    private var statusMessages = {
         "100": "Continue", "200": "OK", "201": "Created", "202": "Accepted", "204": "No Content", 
         "205": "Reset Content", "206": "Partial Content", "301": "Moved Permanently", "302": "Moved Temporarily", 
         "304": "Not Modified", "305": "Use Proxy", "307": "Temporary Redirect", "400": "Bad Request", 
         "401": "Unauthorized", "402": "Payment Required", "403": "Forbidden", "404": "Not Found", 
         "405": "Method Not Allowed", "406": "Not Acceptable", "408": "Request Time-out", "409": "Conflict", 
         "410": "Length Required", "411": "Length Required", "413": "Request Entity Too Large", 
         "414": "Request-URI Too Large", "415": "Unsupported Media Type", "416": "Requested Range Not Satisfiable", 
         "417": "Expectation Failed", "500": "Internal Server Error", "501": "Not Implemented", "502": "Bad Gateway", 
         "503": "Service Unavailable", "504": "Gateway Time-out", "505": "Http Version Not Supported", 
         "507": "Insufficient Storage",
    }

    function Response(conn: Socket, request: Request) {
        this.conn = conn
        this.request = request
    }

    function addHeader(field: String, value: String)
        headers[field] ||= value

    function setHeader(field: String, value: String)
        headers[field] = value

    function addHeaders() {
        headers["Date"] ||= Date().toUTCString())
        headers["ContentLength"] ||= 0
        headers["Content-Type"] ||= "text/html"
        headers["Connection"] ||= "text/html")
        if (--request.keepAlive > 0) {
            resp.setHeader("Connection", "keep-alive")
            resp.SetHeader("Keep-Alive", "timeout=" + timeout + ", max=", keepAlive)
        }
        // addHeader("ETag", "")
    }

    function writeHeaders() {
        addHeaders()
        msg = statusMessages[status] || "Error"
        conn.write(protocol + " " + status + " " + msg + "\r\n")
        for (header in headers) {
            conn.write(header + ":" + headers[header] + "\r\n")
        }
        conn.write("\r\n")
    }

    function sendFile(uri: String) {
        path = documents.join(uri)
        if (!exists(path)) {
            throw "Can't open: " + path
        }
        file = path.open()
        while (file.read(data) >= 0) {
            conn.write(data)
        }
    }

}

class Request {
    private var conn: Socket
    var keepAlive: Number = Number.MaxValue
    public var contentLength: Number
    public var headers: Object = {}
    public var method: String
    public var protocol: String
    public var resp: Response
    public var uri: String

    function Request(conn: Socket) {
        this.conn = conn
    }

    function serve(conn: Socket) {
        ts = new TextStream(conn)
        parseFirst(ts.readLine())
        while ((line = ts.readLine() != "") {
            parseHeader(line)
        }
        parseContent()
        validate()
        respond()
    }

    function parseFirst(line: String) {
        [method, uri, protocol] = line.split(" ")
        if (protocol == "HTTP/1.0") {
            req.keepAlive = 0
        } else if (protocol != "HTTP/1.1") {
            throw "Bad protocol"
        }
        if (methods[method] == undefined) {
            throw "Bad method"
        }
    }

    function parseHeader(line: String) {
        parts = line.split(":")
        [key,value] = lines.split(":")
        headers[key] = value
        switch (key) {
        case "Content-Length":
            contentLength = value cast Number
        }
    }

    function parseContent() {
        //  TODO
    }

    function validate() {
        //  TODO - validate URL and headers
    }

    function respond() {
        resp = new Response(conn)
        resp.setHeader("Content-Length", path.size())
        resp.writeHeaders()
        resp.sendFile(path)
    }
}

class Server {
    private var documents: Path
    private var endpoint: Socket
    private var port: Number
    private var ip: String

    function Server(path: String, port: Number, ip: String = "") {
        documents = Path(path)
        this.port = port
        this.ip = ip
        endpoint = new Socket
        endpoint.async = true
    }

    function run() {
        endpoint.on("readable", function(event, lsock) {
            process(endpoint.accept())
        })
        endpoint.listen(port, ip)
    }

    function process(conn: Socket) {
        try {
            req = new Request(conn)
            req.serve()
            if (req.keepAlive-- > 0) {
                conn.on("readable", process)
            } else {
                conn.close()
            }
        } catch (e) {
            error("Error serving request: " + e)
            conn.close()
        }
    }
}

var cmd = CmdArgs([])
var server = new Server(cmd.args[0] || 80, cmd.args[1] || ".")
server.run()
