/*
    Demonstrate the Socket API. This is a mini (non-production) HTTP server.
    NOTE: Real web servers support Chunking, Ranged requests, POST, PUT, DELETE etc.
    This web server just responds to simple GET requests.

    ejs server.es 80 [documents]
 */

class Response {
    private var conn: Conn
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

    function Response(conn: Conn, request: Request) {
        this.conn = conn
        this.request = request
    }

    function addHeader(field: String, value: String)
        headers[field] ||= value

    function setHeader(field: String, value: String)
        headers[field] = value

    function addHeaders() {
        headers["Date"] ||= Date().toUTCString()
        headers["Content-Length"] ||= 0
        headers["Content-Type"] ||= "text/html"
        if (conn.keepAlive > 0) {
            headers["Connection"] = "keep-alive"
            headers["Keep-Alive"] = "timeout=" + conn.timeout + ", max=" + conn.keepAlive
        } else {
            headers["Connection"] = close
        }
    }

    function writeHeaders() {
        addHeaders()
        msg = statusMessages[status] || "Error"
        App.log.debug(3, "  " + request.protocol + " " + status)
        conn.write(request.protocol + " " + status + " " + msg + "\r\n")
        for (header in headers) {
            conn.write(header + ":" + headers[header] + "\r\n")
        }
        conn.write("\r\n")
    }

    function sendFile(path: Path) {
        file = path.open()
        let data = new ByteArray
        while (file.read(data) > 0) {
            conn.write(data)
        }
    }

}

class Request {
    static var methods = { "DELETE", "GET", "OPTIONS", "POST", "PUT", "TRACE" }
    private var conn: Conn
    public var contentLength: Number
    public var headers: Object = {}
    public var method: String
    public var protocol: String
    public var resp: Response
    public var uri: String

    function Request(conn: Conn)
        this.conn = conn

    function serve(input: TextStream) {
        parseFirst(input.readLine())
        while ((line = input.readLine()) != "") {
            parseHeader(line)
        }
        respond()
    }

    function parseFirst(line) {
        App.log.debug(4, line)
        [method, uri, protocol] = line.split(" ")
        if (protocol == "HTTP/1.0") {
            conn.keepAlive = 0
        } else if (protocol != "HTTP/1.1") {
            throw "Bad protocol"
        }
        if (methods[method] === undefined) {
            throw "Bad method"
        }
        App.log.debug(2, method + " " + uri)
    }

    function parseHeader(line: String) {
        App.log.debug(4, line)
        [key, value] = line.split(":")
        headers[key] = value
        switch (key) {
        case "Content-Length":
            contentLength = value cast Number
        }
    }

    function respond() {
        resp = new Response(conn, this)
        let path: Path = conn.server.documents.join(uri.slice(1))
        if (path.isDir) {
            path = path.join("index.html")
        }
        App.log.debug(4, "Path " + path)
        if (!path.exists) {
            throw "Can't open: " + path
        }
        resp.setHeader("Content-Length", path.size)
        resp.writeHeaders()
        resp.sendFile(path)
    }
}


class Conn {
    var data: ByteArray
    var keepAlive: Number = 500
    var server: Server
    var sock: Socket
    var timeout: Number = 60

    function Conn(server: Server, sock: Socket) {
        this.server = server
        this.sock = sock 
        data = new ByteArray
        req = new Request(this)
        sock.on("readable", function() {
            if (sock.read(data, -1) && data.toString().contains("\r\n\r\n")) {
                req.serve(new TextStream(data))
                data.reset()
                if (keepAlive-- > 0) return
            }
            sock.close()
        })
    }

    function write(...data) {
        try {
            sock.write(...data)
        } catch {}
    }
}


class Server {
    var documents: Path
    var endpoint: Socket
    var port: Number = 80
    var ip: String = "*"

    function Server(address: String, documents: Path = ".") {
        this.documents = documents
        if (address.contains(":")) {
            [ip, port] = address.split(":")
        } else if (address.isDigit) {
            port = address
        } else {
            ip = address
        }
        endpoint = new Socket
        endpoint.async = true
    }

    function start() {
        endpoint.on("accept", function(event, lsock) {
            Conn(this, endpoint.accept())
        })
        App.log.debug(0, "Listening to: " + ip + ":" + port)
        endpoint.listen(port, ip)
    }

}

var port = App.args[1] || 80
var documents = App.args[2] || "."
var server = new Server(port, documents)
server.start()
App.run()
